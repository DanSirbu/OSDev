#pragma once

void (*signal(int sig, void (*func)(int)))(int);