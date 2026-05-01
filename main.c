#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/extensions/record.h>
#include <X11/extensions/XTest.h>
#include "emojis.h"

Display *ctrl_disp = NULL;
Display *data_disp = NULL;
XRecordContext context;

char buffer[256] = {0};
int buf_len = 0;
int is_typing = 0;

// Levenshtein distance for spell checking
int levenshtein(const char *s, int ls, const char *t, int lt) {
    if (!ls) return lt;
    if (!lt) return ls;
    int d1[lt + 1];
    int d2[lt + 1];
    int *prev = d1;
    int *curr = d2;
    for (int i = 0; i <= lt; i++) prev[i] = i;
    for (int i = 1; i <= ls; i++) {
        curr[0] = i;
        for (int j = 1; j <= lt; j++) {
            int a = prev[j] + 1;
            int b = curr[j - 1] + 1;
            int c = prev[j - 1] + (s[i - 1] == t[j - 1] ? 0 : 1);
            int min = a < b ? a : b;
            curr[j] = min < c ? min : c;
        }
        int *temp = prev;
        prev = curr;
        curr = temp;
    }
    return prev[lt];
}

void trigger_emoji(const char *emoji, int backspaces_to_send, const char *suffix) {
    is_typing = 1;
    if (fork() == 0) {
        char cmd[1024] = {0};
        char bs_str[512] = {0};
        for(int b = 0; b < backspaces_to_send; b++) {
            strcat(bs_str, "BackSpace ");
        }
        
        // Wait a tiny bit allowing Xorg to catch up
        sprintf(cmd, "sleep 0.05 && xdotool key %s type '%s'", bs_str, emoji);
        system(cmd);
        
        if (suffix && strlen(suffix) > 0) {
            char suf_cmd[256] = {0};
            sprintf(suf_cmd, "sleep 0.02 && xdotool key %s", suffix);
            system(suf_cmd);
        }
        exit(0);
    }
    
    // reset buffer
    buf_len = 0;
    buffer[0] = '\0';
    is_typing = 0;
}

void xrecord_callback(XPointer ptr, XRecordInterceptData *data) {
    (void)ptr;
    if (data->category == XRecordFromServer) {
        int event_type = data->data[0] & 0x7F;
        
        // KeyPress
        if (event_type == KeyPress) {
            if (is_typing) {
                XRecordFreeData(data);
                return;
            }

            xEvent *xE = (xEvent *)data->data;
            XKeyEvent xkey;
            xkey.type = KeyPress;
            xkey.display = ctrl_disp;
            xkey.window = DefaultRootWindow(ctrl_disp);
            xkey.root = DefaultRootWindow(ctrl_disp);
            xkey.subwindow = None;
            xkey.time = xE->u.keyButtonPointer.time;
            xkey.x = xkey.y = xkey.x_root = xkey.y_root = 1;
            xkey.state = xE->u.keyButtonPointer.state;
            xkey.keycode = xE->u.u.detail;
            xkey.same_screen = True;

            KeySym keysym;
            char chars[8] = {0};
            int len = XLookupString(&xkey, chars, sizeof(chars), &keysym, NULL);

            if (len > 0 && chars[0] >= 32 && chars[0] <= 126) {
                if (buf_len < (int)sizeof(buffer) - 1) {
                    char c = chars[0];
                    
                    // If it is a delimiter
                    if (c == ' ' || keysym == XK_Return || keysym == XK_Tab) {
                        // Check if we have a word starting with ..
                        // Find last ".."
                        char *last_dot = strstr(buffer, "..");
                        if (last_dot) {
                            // Find any spaces after the ".."
                            if (strchr(last_dot, ' ') == NULL) {
                                // We are typing a word that starts with ..
                                // Perform fuzzy match!
                                int word_len = strlen(last_dot);
                                if (word_len > 2) {
                                    int best_dist = 999;
                                    int best_idx = -1;
                                    
                                    for (size_t i = 0; i < EMOJI_MAPPINGS_COUNT; i++) {
                                        const char *alias = EMOJI_MAPPINGS[i].alias;
                                        int dist = levenshtein(last_dot, word_len, alias, strlen(alias));
                                        
                                        // Prefix match gets a bonus
                                        if (strncmp(last_dot, alias, word_len) == 0) {
                                            dist -= 2;
                                        }

                                        if (dist < best_dist) {
                                            best_dist = dist;
                                            best_idx = i;
                                        }
                                    }
                                    
                                    // Threshold for autocorrect: 
                                    // Since we require '..', we are more lenient.
                                    // Max distance allowed is roughly heavily dependent on length.
                                    int threshold = word_len / 2;
                                    if (best_dist <= threshold || best_dist <= 2) {
                                        const char *key_to_send_after = NULL;
                                        if (keysym == XK_Return) key_to_send_after = "Return";
                                        else if (keysym == XK_Tab) key_to_send_after = "Tab";
                                        else if (c == ' ') key_to_send_after = "space";

                                        // We need to delete the typed word + the delimiter just pressed
                                        trigger_emoji(EMOJI_MAPPINGS[best_idx].emoji, word_len + 1, key_to_send_after);
                                        XRecordFreeData(data);
                                        return;
                                    }
                                }
                            }
                        }
                        
                        // If no fuzzy match, add the delimiter to buffer
                        buffer[buf_len++] = c;
                        buffer[buf_len] = '\0';
                    } else {
                        // Regular printable char
                        buffer[buf_len++] = c;
                        buffer[buf_len] = '\0';
                    }
                }
            } else if (keysym == XK_BackSpace) {
                if (buf_len > 0) buffer[--buf_len] = '\0';
            } else if (keysym == XK_Left || keysym == XK_Right || keysym == XK_Up || keysym == XK_Down || keysym == XK_Escape) {
                buf_len = 0;
                buffer[0] = '\0';
            } else if (keysym == XK_Return || keysym == XK_Tab) {
                 char *last_dot = strstr(buffer, "..");
                 if (last_dot && strchr(last_dot, ' ') == NULL) {
                     int word_len = strlen(last_dot);
                     if (word_len > 2) {
                         int best_dist = 999;
                         int best_idx = -1;
                         for (size_t i = 0; i < EMOJI_MAPPINGS_COUNT; i++) {
                             const char *al = EMOJI_MAPPINGS[i].alias;
                             int dist = levenshtein(last_dot, word_len, al, strlen(al));
                             if (strncmp(last_dot, al, word_len) == 0) dist -= 2;
                             if (dist < best_dist) { best_dist = dist; best_idx = i; }
                         }
                         if (best_dist <= word_len / 2 || best_dist <= 2) {
                             const char *suf = (keysym == XK_Return) ? "Return" : "Tab";
                             trigger_emoji(EMOJI_MAPPINGS[best_idx].emoji, word_len + 1, suf);
                             XRecordFreeData(data);
                             return;
                         }
                     }
                 }
                 buffer[buf_len++] = ' ';
                 buffer[buf_len] = '\0';
            }
        }
    }
    XRecordFreeData(data);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Starting Emojix...\n");
    printf("Loaded %zu emojis.\n", EMOJI_MAPPINGS_COUNT);

    ctrl_disp = XOpenDisplay(NULL);
    data_disp = XOpenDisplay(NULL);

    if (!ctrl_disp || !data_disp) {
        fprintf(stderr, "Unable to open X display.\n");
        return 1;
    }

    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *range = XRecordAllocRange();
    if (!range) {
        fprintf(stderr, "Failed to allocate XRecordRange.\n");
        return 1;
    }

    range->device_events.first = KeyPress;
    range->device_events.last  = KeyRelease;

    context = XRecordCreateContext(ctrl_disp, 0, &clients, 1, &range, 1);
    if (!context) {
        fprintf(stderr, "Failed to create XRecordContext.\n");
        return 1;
    }

    XFree(range);
    XSync(ctrl_disp, False);

    printf("Listening for emoji aliases globally. Fuzzy prediction enabled on Space/Tab/Enter.\n");
    
    if (!XRecordEnableContext(data_disp, context, xrecord_callback, NULL)) {
        fprintf(stderr, "Failed to enable XRecordContext.\n");
        return 1;
    }

    XRecordFreeContext(ctrl_disp, context);
    XCloseDisplay(data_disp);
    XCloseDisplay(ctrl_disp);
    return 0;
}
