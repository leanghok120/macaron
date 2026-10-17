#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <Imlib2.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>

int width = 250;
int height = 250;
int x = 25;
int y = 25;

Display *dpy;
Window win = None;
static volatile sig_atomic_t running = 1;

static void handle_signal(int signum) {
  (void)signum;
  running = 0;
}
Visual *vis;
int depth;
Colormap cm;

void help(const char *msg) {
  printf("usage: %s [options] [file]\nOPTIONS:\n-x\n\tset x position\n-y\n\tset y position\n-w\n\tset width of widget\n-h\n\tset height of widget\n", msg);
  exit(1);
}

void parseargs(int argc, char **argv) {
  int option;

  if (argc < 2) {
    help(argv[0]);
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
        help(argv[0]);
        break;
    }
  }

  if (optind >= argc) {
    help(argv[0]);
  }
}

void init() {
  dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    fprintf(stderr, "Error: Cannot open X display\n");
    exit(EXIT_FAILURE);
  }
  win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), x, y, width, height, 0, 0, 0);

  vis = DefaultVisual(dpy, DefaultScreen(dpy));
  depth = DefaultDepth(dpy, DefaultScreen(dpy));
  cm = DefaultColormap(dpy, DefaultScreen(dpy));

  XSelectInput(dpy, win, ExposureMask);
  XSetWindowBackgroundPixmap(dpy, win, None);

  XClassHint *classhint = XAllocClassHint();
  if (classhint != NULL) {
    classhint->res_class = "macaron";
    classhint->res_name = "macaron";
    XSetClassHint(dpy, win, classhint);
    XFree(classhint);
  }

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

void cleanup(Imlib_Image image) {
  if (image != NULL) {
    imlib_context_set_image(image);
    imlib_free_image();
  }
  if (dpy != NULL) {
    if (win != None) {
      XDestroyWindow(dpy, win);
    }
    XCloseDisplay(dpy);
  }
}

int main(int argc, char **argv) {
  parseargs(argc, argv);
  init();

  struct sigaction sa = {0};
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  // load image
  Imlib_Image image;
  image = imlib_load_image(argv[optind]);
  if (image == NULL) {
    fprintf(stderr, "Error: Failed to load image\n");
    cleanup(NULL);
    return EXIT_FAILURE;
  }
  imlib_context_set_image(image);

  // draw image
  while (running) {
    while (running && XPending(dpy) > 0) {
      XEvent ev;
      XNextEvent(dpy, &ev);
      if (ev.type == Expose) {
        imlib_render_image_on_drawable_at_size(0, 0, width, height);
        XFlush(dpy);
      }
    }
    if (!running) break;

    struct pollfd pfd = { .fd = ConnectionNumber(dpy), .events = POLLIN };
    int result = poll(&pfd, 1, -1);
    if (result < 0 && errno != EINTR) {
      perror("poll");
      break;
    }
    if (result > 0 && (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))) {
      break;
    }
  }

  cleanup(image);
  return 0;
}
