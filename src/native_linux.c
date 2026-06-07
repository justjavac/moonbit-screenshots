#include "native_stub.h"

#if defined(__linux__)
#include <dlfcn.h>
#include <stdlib.h>

typedef struct _XDisplay MoonBitScreenshotsDisplay;
typedef unsigned long MoonBitScreenshotsXid;
typedef MoonBitScreenshotsXid MoonBitScreenshotsWindow;
typedef MoonBitScreenshotsXid MoonBitScreenshotsDrawable;
typedef char *MoonBitScreenshotsXPointer;
typedef struct _MoonBitScreenshotsXImage MoonBitScreenshotsXImage;

struct _MoonBitScreenshotsXImage {
  int width;
  int height;
  int xoffset;
  int format;
  char *data;
  int byte_order;
  int bitmap_unit;
  int bitmap_bit_order;
  int bitmap_pad;
  int depth;
  int bytes_per_line;
  int bits_per_pixel;
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
  MoonBitScreenshotsXPointer obdata;
  struct {
    MoonBitScreenshotsXImage *(*create_image)(void);
    int (*destroy_image)(MoonBitScreenshotsXImage *);
    unsigned long (*get_pixel)(MoonBitScreenshotsXImage *, int, int);
    int (*put_pixel)(MoonBitScreenshotsXImage *, int, int, unsigned long);
    MoonBitScreenshotsXImage *(*sub_image)(
      MoonBitScreenshotsXImage *,
      int,
      int,
      unsigned int,
      unsigned int
    );
    int (*add_pixel)(MoonBitScreenshotsXImage *, long);
  } f;
};

static uint8_t moonbit_screenshots_mask_to_u8(
  unsigned long pixel,
  unsigned long mask
) {
  if (mask == 0) {
    return 0;
  }
  int shift = 0;
  while (((mask >> shift) & 1UL) == 0UL) {
    shift += 1;
  }
  unsigned long max = mask >> shift;
  unsigned long value = (pixel & mask) >> shift;
  return (uint8_t)((value * 255UL + max / 2UL) / max);
}

static void *moonbit_screenshots_open_x11(void) {
  void *x11 = dlopen("libX11.so.6", RTLD_LAZY);
  if (x11 == NULL) {
    x11 = dlopen("libX11.so", RTLD_LAZY);
  }
  return x11;
}

int moonbit_screenshots_capture_linux(
  int32_t kind,
  int32_t display_index,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height,
  uint64_t window_id,
  moonbit_bytes_t output_path,
  moonbit_bytes_t output_path_wide
) {
  (void)display_index;
  (void)output_path_wide;
  void *x11 = moonbit_screenshots_open_x11();
  if (x11 == NULL) {
    return -1;
  }
  MoonBitScreenshotsDisplay *(*open_display)(const char *) = dlsym(x11, "XOpenDisplay");
  int (*close_display)(MoonBitScreenshotsDisplay *) = dlsym(x11, "XCloseDisplay");
  MoonBitScreenshotsWindow (*default_root_window)(MoonBitScreenshotsDisplay *) =
    dlsym(x11, "XDefaultRootWindow");
  int (*get_geometry)(
    MoonBitScreenshotsDisplay *,
    MoonBitScreenshotsDrawable,
    MoonBitScreenshotsWindow *,
    int *,
    int *,
    unsigned int *,
    unsigned int *,
    unsigned int *,
    unsigned int *
  ) = dlsym(x11, "XGetGeometry");
  MoonBitScreenshotsXImage *(*get_image)(
    MoonBitScreenshotsDisplay *,
    MoonBitScreenshotsDrawable,
    int,
    int,
    unsigned int,
    unsigned int,
    unsigned long,
    int
  ) = dlsym(x11, "XGetImage");
  if (
    open_display == NULL ||
    close_display == NULL ||
    default_root_window == NULL ||
    get_geometry == NULL ||
    get_image == NULL
  ) {
    return -1;
  }

  MoonBitScreenshotsDisplay *display = open_display(NULL);
  if (display == NULL) {
    return -1;
  }
  MoonBitScreenshotsWindow root = default_root_window(display);
  MoonBitScreenshotsDrawable drawable = root;
  int source_x = 0;
  int source_y = 0;
  unsigned int capture_width = 0;
  unsigned int capture_height = 0;
  MoonBitScreenshotsWindow ignored_root = 0;
  int ignored_x = 0;
  int ignored_y = 0;
  unsigned int ignored_border = 0;
  unsigned int ignored_depth = 0;

  if (kind == MOONBIT_SCREENSHOTS_TARGET_WINDOW) {
    drawable = (MoonBitScreenshotsDrawable)window_id;
    if (
      get_geometry(
        display,
        drawable,
        &ignored_root,
        &ignored_x,
        &ignored_y,
        &capture_width,
        &capture_height,
        &ignored_border,
        &ignored_depth
      ) == 0
    ) {
      close_display(display);
      return -1;
    }
  } else if (kind == MOONBIT_SCREENSHOTS_TARGET_AREA) {
    drawable = root;
    source_x = x;
    source_y = y;
    capture_width = (unsigned int)(width < 1 ? 1 : width);
    capture_height = (unsigned int)(height < 1 ? 1 : height);
  } else {
    drawable = root;
    if (
      get_geometry(
        display,
        root,
        &ignored_root,
        &ignored_x,
        &ignored_y,
        &capture_width,
        &capture_height,
        &ignored_border,
        &ignored_depth
      ) == 0
    ) {
      close_display(display);
      return -1;
    }
  }

  if (capture_width == 0 || capture_height == 0) {
    close_display(display);
    return -1;
  }
  MoonBitScreenshotsXImage *image = get_image(
    display,
    drawable,
    source_x,
    source_y,
    capture_width,
    capture_height,
    ~0UL,
    2
  );
  if (image == NULL || image->f.get_pixel == NULL) {
    if (image != NULL && image->f.destroy_image != NULL) {
      image->f.destroy_image(image);
    }
    close_display(display);
    return -1;
  }
  size_t pixel_count = (size_t)capture_width * (size_t)capture_height;
  uint8_t *rgba = (uint8_t *)malloc(pixel_count * 4u);
  if (rgba == NULL) {
    if (image->f.destroy_image != NULL) {
      image->f.destroy_image(image);
    }
    close_display(display);
    return -1;
  }
  for (unsigned int yy = 0; yy < capture_height; yy++) {
    for (unsigned int xx = 0; xx < capture_width; xx++) {
      unsigned long pixel = image->f.get_pixel(image, (int)xx, (int)yy);
      size_t offset = ((size_t)yy * (size_t)capture_width + (size_t)xx) * 4u;
      rgba[offset + 0u] = moonbit_screenshots_mask_to_u8(pixel, image->red_mask);
      rgba[offset + 1u] = moonbit_screenshots_mask_to_u8(pixel, image->green_mask);
      rgba[offset + 2u] = moonbit_screenshots_mask_to_u8(pixel, image->blue_mask);
      rgba[offset + 3u] = 255;
    }
  }
  if (image->f.destroy_image != NULL) {
    image->f.destroy_image(image);
  }
  close_display(display);
  int result = moonbit_screenshots_write_png_rgba(
    output_path,
    NULL,
    (int)capture_width,
    (int)capture_height,
    rgba
  );
  free(rgba);
  return result;
}

#else
int moonbit_screenshots_capture_linux(
  int32_t kind,
  int32_t display_index,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height,
  uint64_t window_id,
  moonbit_bytes_t output_path,
  moonbit_bytes_t output_path_wide
) {
  (void)kind;
  (void)display_index;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)window_id;
  (void)output_path;
  (void)output_path_wide;
  return -1;
}
#endif
