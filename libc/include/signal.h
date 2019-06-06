#pragma once

void (*signal(int sig, void (*func)(int)))(int);

//From https://www.rpi.edu/dept/cis/software/g77-mingw32/include/signal.h

#define SIGINT 2 /* Interactive attention */
#define SIGILL 4 /* Illegal instruction */
#define SIGFPE 8 /* Floating point error */
#define SIGSEGV 11 /* Segmentation violation */
#define SIGTERM 15 /* Termination request */
#define SIGBREAK 21 /* Control-break */
#define SIGABRT 22 /* Abnormal termination (abort) */

#define NSIG 23 /* maximum signal number + 1 */

#define SIG_DFL (0)
#define SIG_IGN (1)
#define SIG_ERR (-1)