#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WIDTH 230
#define HEIGHT 230

Display *dpy;
Window win;
Visual *vis;
int depth;
Colormap cm;

void init() {
  dpy = XOpenDisplay(NULL);
  win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 25, 25, WIDTH, HEIGHT, 0, 0, 0);

  vis = DefaultVisual(dpy, DefaultScreen(dpy));
  depth = DefaultDepth(dpy, DefaultScreen(dpy));
  cm = DefaultColormap(dpy, DefaultScreen(dpy));

  XSelectInput(dpy, win, ExposureMask);
  XSetWindowBackgroundPixmap(dpy, win, None);

  XSetWindowAttributes attrs;
  attrs.override_redirect = True;
  XChangeWindowAttributes(dpy, win, CWOverrideRedirect, &attrs);
  XMapWindow(dpy, win);
  XLowerWindow(dpy, win);

  // setup Imlib2
  imlib_context_set_display(dpy);
  imlib_context_set_visual(vis);
  imlib_context_set_colormap(cm);
  imlib_context_set_drawable(win);
}

void cleanup() {
  imlib_free_image();
  XFreeColormap(dpy, cm);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: %s [file]\n", argv[0]);
    return 1;
  }

  init();

  // load image
  Imlib_Image image;
  image = imlib_load_image(argv[1]);
  if (image == NULL) {
    printf("failed to load image");
    exit(1);
  }
  imlib_context_set_image(image);

  // draw image
  XEvent ev;
  while (1) {
    XNextEvent(dpy, &ev);
    if (ev.type == Expose) {
      imlib_render_image_on_drawable_at_size(0, 0, WIDTH, HEIGHT);
      XFlush(dpy);
    }
  }

  cleanup();
  return 0;
}
