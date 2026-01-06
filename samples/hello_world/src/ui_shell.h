#ifndef UI_SHELL_
#define UI_SHELL_
/* Shell UI */
void ui_shell_init(void);
void shell_feed_char(char c);
void shell_feed_test(char* c);
int startShell(void);
void checkShellReady(void);
// extern struct shell shell_ui;
#endif
