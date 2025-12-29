#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <Imlib2.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int width = 250;
int height = 250;
int x = 25;
int y = 25;

Display *dpy;
Window win;
Visual *vis;
int depth;
Colormap cm;

void parseargs(int argc, char **argv) {
  int option;

  if (argc < 2) {
    printf("usage: %s [options] [file]\n", argv[0]);
    exit(1);
  }

  while ((option = getopt(argc, argv, "x:y:w:h:")) != -1) {
    switch (option) {
      case 'x':
        x = atoi(optarg);
        break;
      case 'y':
        y = atoi(optarg);
        break;
      case 'w':
        width = atoi(optarg);
        break;
      case 'h':
        height = atoi(optarg);
        break;
      case '?':
        printf("usage: %s [options] [file]\n", argv[0]);
        exit(1);
        break;
    }
  }

  if (optind >= argc) {
    printf("usage: %s [options] [file]\n", argv[0]);
    exit(1);
  }
}

void init() {
  dpy = XOpenDisplay(NULL);
  win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), x, y, width, height, 0, 0, 0);

  vis = DefaultVisual(dpy, DefaultScreen(dpy));
  depth = DefaultDepth(dpy, DefaultScreen(dpy));
  cm = DefaultColormap(dpy, DefaultScreen(dpy));

  XSelectInput(dpy, win, ExposureMask);
  XSetWindowBackgroundPixmap(dpy, win, None);

  XClassHint *classhint = XAllocClassHint();
  classhint->res_class = "macaron";
  classhint->res_name = "macaron";
  XSetClassHint(dpy, win, classhint);

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
  parseargs(argc, argv);
  init();

  // load image
  Imlib_Image image;
  image = imlib_load_image(argv[optind]);
  if (image == NULL) {
    printf("failed to load image\n");
    exit(1);
  }
  imlib_context_set_image(image);

  // draw image
  XEvent ev;
  while (1) {
    XNextEvent(dpy, &ev);
    if (ev.type == Expose) {
      imlib_render_image_on_drawable_at_size(0, 0, width, height);
      XFlush(dpy);
    }
  }

  cleanup();
  return 0;
}
