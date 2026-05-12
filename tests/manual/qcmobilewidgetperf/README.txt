This QCanvasPainterWidget (so QRhiWidget) application is intended for mobile
devices, Android, first and foremost. Also fine for desktop.

It is only really some filled and strokes rectangles, covering the entire screen (so
screen resolution matters), but it is sufficient to present enough load on
weaker devices, or on powerful devices with higher resolutions and refresh
rates. Example: on a Galaxy Tab S9 and its 120 Hz screen, the expectation is 120
FPS with this app, but the drop in Vulkan frame rates (as opposed to GLES) is
already visible (this seems to be solved by
https://codereview.qt-project.org/c/qt/qtbase/+/734463 in qtbase).

Also useful for testing the screen rotation related issues (e.g. to demonstrate
occasional heavy drops in Vulkan performance when in a non-natural orientation,
probably due to https://developer.android.com/games/optimize/vulkan-prerotation )

To make OpenGL ES vs. Vulkan testing easy on Android, a widget-based dialog is
shown at startup to choose the graphics API to be used.

While the app has an FPS counter, it is highly recommended to be used together
with GPUWatch, if on a Samsung device (toggleable in system Developer settings),
to get the overlay showing CPU load, GPU load, and FPS from the system as well
and get a better picture of what's going on.
