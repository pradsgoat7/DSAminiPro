#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ncurses.h> 

#define MAX_URL_LENGTH 3000

struct Node {
    char url[MAX_URL_LENGTH]; 
    struct Node * next;
    struct Node * prev;
};

struct browserHistory {
    struct Node * head;
    struct Node * tail;
    struct Node * current;
};


//function 1: visiting new page loml
void visitNewPage(struct browserHistory* history, const char* url) {
    struct Node * newNode;
    newNode = (struct Node *)malloc(sizeof(struct Node));
    
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    strncpy(newNode->url, url, MAX_URL_LENGTH - 1);
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->url[MAX_URL_LENGTH - 1] = '\0';        //  \0 works as a full stop 

    //history is completely empty -> no tabs are opened
    if(history -> current == NULL) {
        history -> head = newNode;
        history -> tail = newNode;
        history -> current = newNode;
        printf("Visited a new page: %s\n", url);
        return;
    } 

    //A -> B -> C -> D       A -> B -> X            C AND D ARE DELETED FROM HISTORY
    if(history -> current -> next != NULL) {
        printf("Deleting forward history\n");
        struct Node * temp = history -> current -> next;

        while(temp != NULL) {
            struct Node * nodeToDelete = temp;
            temp = temp -> next;
            printf("%s Page deleted\n", nodeToDelete -> url);
            free(nodeToDelete);
        }
    }
    history -> current -> next = newNode;
    newNode -> prev = history -> current;

    history -> current = newNode;
    history -> tail = newNode;

    printf("Visited new page: %s\n", url);
}

//function 2: going back
void goBack(struct browserHistory* history) {
    if (history->current == NULL || history->current->prev == NULL) {
        printf("Cannot go back. This is the oldest page in the history.\n");
        return;
    } else {
        history -> current = history -> current -> prev;
        printf("Going Back, Current Page now is: %s\n", history -> current -> url);
    }
}

//function 3: going forward 
void goForward(struct browserHistory* history) {
    if(history -> current == NULL || history -> current -> next == NULL) {
        printf("Cant go Forward, This is the Latest Page\n");
        return;
    } else {
        history -> current = history -> current -> next;
        printf("Going Forward, Current Page now is: %s\n", history -> current -> url);
    }
}


//function 4: displays history
void displayHistory(struct browserHistory* history) {
    printf("-------BROWSER HISTORY-------\n");
    if(history -> head == NULL){
        printf("History is Empty\n");
        printf("-----------------------------\n");
        return;
    } 
        struct Node * ptr = history -> head;
        while (ptr != NULL) {
            printf("%s", ptr->url);

            if(ptr == history -> current) {
                printf("    <--- YOU ARE HERE");
            }
            printf("\n");
            ptr = ptr->next;
        }
    printf("-----------------------------\n");
}


//function 5: deletes entire history
//didnt understand this achese ;-;
void deleteEntireHistory(struct browserHistory* history) {
    struct Node * ptr = history -> head;
    struct Node * nodeToDelete;
    
    while(ptr != NULL) {
        nodeToDelete = ptr;
        ptr = ptr -> next;
        free(nodeToDelete);
    }
    history -> head = NULL;
    history -> tail = NULL;
    history -> current = NULL;

    printf("\nBrowser history cleared successfully.\n");
}

void deleteParticularPage(struct browserHistory* history, const char* urlToDelete) {

    struct Node * ptr = history -> head;
    struct Node * nodeToDelete;

 //strcmp returns 0 when the 2 strings are same, it is not like true false
    while(ptr != NULL && strcmp(ptr -> url, urlToDelete) != 0) {       
     ptr = ptr -> next;   
    }
    
    //1.
    if (ptr == NULL) {
        printf("Page not found: %s\n", urlToDelete);
        return;
    }

    //2.
    //move the bookmark to the previous page. 
    //It prevents the current pointer from pointing to something that no longer exists.
    if (history -> current == ptr) {
        history -> current = ptr -> prev;
    }

    if(ptr == history -> head) {
        history -> head = ptr -> next;
        if(history -> head != NULL) {
            history -> head -> prev = NULL;
        }
    }

    else if (ptr == history -> tail) {
        history -> tail = ptr -> prev;
        history -> tail -> next = NULL;
    }

    else {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
    }

    if (history->head == NULL) {
        history->tail = NULL;
        history->current = NULL;
    }

    printf("Successfully deleted page: %s\n", ptr->url);
    free(ptr);
}


//integrating API in the project xD
// The callback function 
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    printf("--> Shortened URL: %s\n", (char *)ptr);
    return size * nmemb;
}

// The new function to call the API
void shortenUrl(const char* longUrl) {
    if (longUrl == NULL) return;
    CURL *curl;
    char apiUrl[MAX_URL_LENGTH + 50]; 
    sprintf(apiUrl, "https://tinyurl.com/api-create.php?url=%s", longUrl);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

void shortenUrlTUI(struct browserHistory* history) {
    if (history->current == NULL) {
        mvprintw(LINES - 2, 1, "No current page to shorten. Press any key...");
        getch();
        return;
    }
    
    // We need to exit ncurses to see the API response from the callback
    endwin();
    printf("Contacting API...\n");
    shortenUrl(history->current->url); // Assuming you have your original shortenUrl function
    
    printf("\nPress any key to return to the menu...");
    getch();
}


void draw_menu(int highlight) {
    clear(); // Clear the screen
    int height, width;
    getmaxyx(stdscr, height, width); // Get screen dimensions

    mvprintw(1, (width - 25) / 2, "Browser History Navigator"); // Title
    box(stdscr, 0, 0); // Draw a border

    char* choices[] = {
        "1. Visit a new page",
        "2. Go back",
        "3. Go forward",
        "4. Display full history",
        "5. Delete a specific page",
        "6. Clear entire history",
        "7. Shorten Current URL",
        "8. Exit"
    };
    int num_choices = 8;

    for (int i = 0; i < num_choices; i++) {
        int y = (height / 2) - (num_choices / 2) + i;
        int x = (width / 2) - (strlen(choices[i]) / 2);
        
        if (i == highlight) {
            attron(A_REVERSE); // Highlight the current choice
        }
        mvprintw(y, x, "%s", choices[i]);
        if (i == highlight) {
            attroff(A_REVERSE); // Turn off highlighting
        }
    }
    refresh();
}

int main() {
    struct browserHistory myBrowser = {NULL, NULL, NULL};
    char url[MAX_URL_LENGTH];

    // --- ncurses setup ---
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int highlight = 0;
    int choice = 0;

    // --- The Main TUI Loop ---
    while(1) {
        draw_menu(highlight);
        int input = getch(); // Wait for a key press

        switch(input) {
            case KEY_UP:
                highlight--;
                if (highlight < 0) highlight = 7; // Wrap around
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight > 7) highlight = 0; // Wrap around
                break;
            case 10: // The Enter key
                choice = highlight + 1;
                break;
            default:
                break;
        }

        // If the user pressed Enter, 'choice' will be set.
        if (choice != 0) {
            // Temporarily stop ncurses to get user text input or show scrolled output
            if (choice == 1 || choice == 4 || choice == 5 || choice == 7) {
                endwin();
                printf("\n");

                switch(choice) {
                    case 1:
                        printf("Enter the URL to visit: ");
                        scanf("%s", url);
                        visitNewPage(&myBrowser, url);
                        break;
                    case 4:
                        displayHistory(&myBrowser);
                        break;
                    case 5:
                        printf("Enter the URL to delete: ");
                        scanf("%s", url);
                        deleteParticularPage(&myBrowser, url);
                        break;
                    case 7:
                        shortenUrlTUI(&myBrowser);
                        break;
                }

                if (choice != 7) { // Don't wait for key press if API already did
                    printf("\nPress any key to return to the menu...");
                    getch(); // Wait for user
                }

                // Re-initialize ncurses to go back to the TUI menu
                initscr();
                cbreak();
                noecho();
                keypad(stdscr, TRUE);

            } else {
                // For functions that don't need text input, just call them
                switch(choice) {
                    case 2: goBack(&myBrowser); break;
                    case 3: goForward(&myBrowser); break;
                    case 6: deleteEntireHistory(&myBrowser); break;
                }
                // Pause to show the result before redrawing the menu
                mvprintw(LINES - 2, 1, "Action complete. Press any key...");
                getch();
            }

            if (choice == 8) { // Exit condition
                break;
            }
            choice = 0; // Reset choice
        }
    }

    // --- Cleanup ---
    endwin();
    deleteEntireHistory(&myBrowser); 
    printf("Exited successfully.\n");
    
    return 0;
}