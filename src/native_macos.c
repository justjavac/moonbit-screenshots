#include "native_stub.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <dlfcn.h>
#include <string.h>

typedef unsigned char MoonBitScreenshotsBoolean;
typedef uint32_t MoonBitScreenshotsDisplayId;
typedef uint32_t MoonBitScreenshotsWindowId;
typedef struct {
  double x;
  double y;
} MoonBitScreenshotsPoint;
typedef struct {
  double width;
  double height;
} MoonBitScreenshotsSize;
typedef struct {
  MoonBitScreenshotsPoint origin;
  MoonBitScreenshotsSize size;
} MoonBitScreenshotsRect;
typedef const void *MoonBitScreenshotsCFTypeRef;
typedef const void *MoonBitScreenshotsCFStringRef;
typedef const void *MoonBitScreenshotsCFURLRef;
typedef void *MoonBitScreenshotsImageRef;
typedef void *MoonBitScreenshotsImageDestinationRef;

static void *moonbit_screenshots_dlsym(void *handle, const char *name) {
  return handle == NULL ? NULL : dlsym(handle, name);
}

static int moonbit_screenshots_save_cgimage(
  MoonBitScreenshotsImageRef image,
  moonbit_bytes_t output_path
) {
  if (image == NULL || output_path == NULL) {
    return -1;
  }
  void *core_foundation = dlopen(
    "/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation",
    RTLD_LAZY
  );
  void *image_io = dlopen(
    "/System/Library/Frameworks/ImageIO.framework/ImageIO",
    RTLD_LAZY
  );
  if (core_foundation == NULL || image_io == NULL) {
    return -1;
  }
  MoonBitScreenshotsCFStringRef (*create_string)(void *, const char *, uint32_t) =
    moonbit_screenshots_dlsym(core_foundation, "CFStringCreateWithCString");
  MoonBitScreenshotsCFURLRef (*create_url)(void *, const uint8_t *, long, int, MoonBitScreenshotsBoolean) =
    moonbit_screenshots_dlsym(core_foundation, "CFURLCreateFromFileSystemRepresentation");
  void (*release)(MoonBitScreenshotsCFTypeRef) =
    moonbit_screenshots_dlsym(core_foundation, "CFRelease");
  MoonBitScreenshotsImageDestinationRef (*create_destination)(
    MoonBitScreenshotsCFURLRef,
    MoonBitScreenshotsCFStringRef,
    size_t,
    void *
  ) = moonbit_screenshots_dlsym(image_io, "CGImageDestinationCreateWithURL");
  void (*add_image)(MoonBitScreenshotsImageDestinationRef, MoonBitScreenshotsImageRef, void *) =
    moonbit_screenshots_dlsym(image_io, "CGImageDestinationAddImage");
  MoonBitScreenshotsBoolean (*finalize)(MoonBitScreenshotsImageDestinationRef) =
    moonbit_screenshots_dlsym(image_io, "CGImageDestinationFinalize");
  if (
    create_string == NULL ||
    create_url == NULL ||
    release == NULL ||
    create_destination == NULL ||
    add_image == NULL ||
    finalize == NULL
  ) {
    return -1;
  }
  MoonBitScreenshotsCFStringRef png_type =
    create_string(NULL, "public.png", 0x08000100u);
  MoonBitScreenshotsCFURLRef url = create_url(
    NULL,
    output_path,
    (long)strlen((const char *)output_path),
    0,
    0
  );
  if (png_type == NULL || url == NULL) {
    if (png_type != NULL) {
      release(png_type);
    }
    if (url != NULL) {
      release(url);
    }
    return -1;
  }
  MoonBitScreenshotsImageDestinationRef destination =
    create_destination(url, png_type, 1, NULL);
  if (destination == NULL) {
    release(url);
    release(png_type);
    return -1;
  }
  add_image(destination, image, NULL);
  MoonBitScreenshotsBoolean ok = finalize(destination);
  release(destination);
  release(url);
  release(png_type);
  return ok ? 0 : -1;
}

static MoonBitScreenshotsRect moonbit_screenshots_make_rect(
  double x,
  double y,
  double width,
  double height
) {
  MoonBitScreenshotsRect rect;
  rect.origin.x = x;
  rect.origin.y = y;
  rect.size.width = width;
  rect.size.height = height;
  return rect;
}

int moonbit_screenshots_capture_macos(
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
  (void)output_path_wide;
  void *app_services = dlopen(
    "/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices",
    RTLD_LAZY
  );
  void *core_foundation = dlopen(
    "/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation",
    RTLD_LAZY
  );
  if (app_services == NULL || core_foundation == NULL) {
    return -1;
  }
  MoonBitScreenshotsDisplayId (*main_display_id)(void) =
    moonbit_screenshots_dlsym(app_services, "CGMainDisplayID");
  int32_t (*get_active_displays)(uint32_t, MoonBitScreenshotsDisplayId *, uint32_t *) =
    moonbit_screenshots_dlsym(app_services, "CGGetActiveDisplayList");
  MoonBitScreenshotsImageRef (*display_image)(MoonBitScreenshotsDisplayId) =
    moonbit_screenshots_dlsym(app_services, "CGDisplayCreateImage");
  MoonBitScreenshotsImageRef (*display_image_for_rect)(
    MoonBitScreenshotsDisplayId,
    MoonBitScreenshotsRect
  ) = moonbit_screenshots_dlsym(app_services, "CGDisplayCreateImageForRect");
  MoonBitScreenshotsImageRef (*window_image)(
    MoonBitScreenshotsRect,
    uint32_t,
    MoonBitScreenshotsWindowId,
    uint32_t
  ) = moonbit_screenshots_dlsym(app_services, "CGWindowListCreateImage");
  void (*release)(MoonBitScreenshotsCFTypeRef) =
    moonbit_screenshots_dlsym(core_foundation, "CFRelease");
  if (
    main_display_id == NULL ||
    display_image == NULL ||
    display_image_for_rect == NULL ||
    window_image == NULL ||
    release == NULL
  ) {
    return -1;
  }

  MoonBitScreenshotsDisplayId display = main_display_id();
  if (kind == MOONBIT_SCREENSHOTS_TARGET_DISPLAY && get_active_displays != NULL) {
    MoonBitScreenshotsDisplayId displays[32];
    uint32_t count = 0;
    if (get_active_displays(32, displays, &count) == 0 && count > 0) {
      uint32_t index = display_index < 0 ? 0u : (uint32_t)display_index;
      display = displays[index < count ? index : 0];
    }
  }

  MoonBitScreenshotsImageRef image = NULL;
  if (kind == MOONBIT_SCREENSHOTS_TARGET_AREA) {
    image = display_image_for_rect(
      display,
      moonbit_screenshots_make_rect(
        (double)x,
        (double)y,
        (double)(width < 1 ? 1 : width),
        (double)(height < 1 ? 1 : height)
      )
    );
  } else if (kind == MOONBIT_SCREENSHOTS_TARGET_WINDOW) {
    image = window_image(
      moonbit_screenshots_make_rect(-1.0e20, -1.0e20, 2.0e20, 2.0e20),
      8u,
      (MoonBitScreenshotsWindowId)window_id,
      0u
    );
  } else {
    image = display_image(display);
  }
  if (image == NULL) {
    return -1;
  }
  int result = moonbit_screenshots_save_cgimage(image, output_path);
  release(image);
  return result;
}

#else
int moonbit_screenshots_capture_macos(
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
