#include <curses.h>
#include <linux/limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include <locale.h>
#include <unistd.h>

#define MAX_SIZE_char 100 
#define ROOM_MIN_SIZE 6
#define MAX_ROOMS 6

FILE* file;
char filename[MAX_SIZE_char];

typedef struct Position {
    int y;
    int x;
} Position;

typedef struct Weapon {
    char symbol;
    int damage;
    int chance;
} Weapon;

typedef struct player {
    char* name;
    char* password;
    char* email;
    Position pos;
    int color;
    int health;
    int gold;
    Weapon weapon;
    int current_floor;
    int hunger;
    int sword_equipped;
    int damage;
} player;

typedef struct Room {
    Position pos;
    int height, width;
    char **layout;
    Position *door;
    Position *gold;
    Position *dark_gold;
    Position *food;
} Room;

typedef struct Monster {
    char symbol;
    int health;
    int damage;
    Position pos;
} Monster;

int dif_num = 0; //easy
int player_color = 1;


void create_account(player* karbar);
void login_account(player* karbar);
void pregame_menu(player* karbar);
void check_pos(Position* newPos, player* hero, char** layout, Monster* monster);
void settings(player* karbar);
Room creare_room(int max_height, int max_width);
void create_rooms(Room rooms[], int num_rooms);
void print_rooms(Room rooms[], int num_rooms);
bool check_overlap(Room room1, Room room2);
void start_game();
void place_player(player* hero, Room room);
void vertical_corridor(int y1, int y2, int x);
void horizontal_corridor(int x1, int x2, int y);
void pickup_sword(player* hero, Position sword_pos);
void combat(player* hero, Monster* monster);


int password_check(player* karbar) {
    int has_capital = 0;
    int has_small = 0;
    int has_num = 0;
    for (int i = 0; i < strlen(karbar->password); ++i) {
        if ('A' <= karbar->password[i] && karbar->password[i] <= 'Z')
            has_capital = 1;
        else if ('a' <= karbar->password[i] && karbar->password[i] <= 'z')
            has_small = 1;
        else if ('0' <= karbar->password[i] && karbar->password[i] <= '9')
            has_num = 1;
    }

    if(!has_capital) {
        clear();
        mvprintw(LINES / 2, COLS / 2, "Password doesn't have capital character");
        return 0;
    }
    else if(!has_small) {
        clear();
        mvprintw(LINES / 2, COLS / 2, "Password doesn't have small character");
        return 0;
    }
    else if(!has_num) {
        clear();
        mvprintw(LINES / 2, COLS / 2, "Password doesn't have number");
        return 0;
    }
    else if(strlen(karbar->password) < 7) {
        clear();
        mvprintw(LINES / 2, COLS / 2, "Password is small");
        return 0;
    }
    else {
        return 1;
    }
}


int email_check(player* karbar) {
    const char *at_sign = strchr(karbar->email, '@');
    if (!at_sign) {
        return 0; 
    } 
    const char *dot = strchr(at_sign, '.'); 
    if (!dot || dot == at_sign + 1) { 
        return 0; 
    }
    if (strlen(dot) < 2) { 
        return 0; 
    }
    for (const char *c = karbar->email; c < at_sign; ++c) { 
        if (!isalnum(*c) && *c != '.' && *c != '_') { 
            return 0; 
        } 
    } 
    if (!isalpha(at_sign[1])) { 
        return 0; 
    } 
    return 1;
}


void create_account(player* karbar) {
    clear();

    karbar->name = (char*)malloc(MAX_SIZE_char * sizeof(char));
    karbar->password = (char*)malloc(MAX_SIZE_char * sizeof(char));
    karbar->email = (char*)malloc(MAX_SIZE_char * sizeof(char));

    initscr();
    mvprintw(LINES / 2, COLS / 2, "Enter your name: ");
    getnstr(karbar->name, MAX_SIZE_char);
    snprintf(filename, MAX_SIZE_char, "%s.txt", karbar->name);
    file = fopen(filename, "r");
    if(file != NULL) {
        fclose(file);
        clear(); 
        mvprintw(LINES / 2, COLS / 2, "Account already exists.");
        refresh();
        getch();
        endwin();
        free(karbar->name);
        free(karbar->password);
        free(karbar->email);
        return;
    }

    mvprintw(LINES / 2 + 1, COLS / 2, "Enter your password: ");
    getnstr(karbar->password, MAX_SIZE_char);
    int checkpass = password_check(karbar);
    if(checkpass == 0) {
        free(karbar->name);
        free(karbar->password);
        free(karbar->email);
        getch(); 
        endwin();
        return;  
    }

    mvprintw(LINES / 2 + 2, COLS / 2, "Enter your email: ");
    getnstr(karbar->email, MAX_SIZE_char);
    int checkmail = email_check(karbar);
    if(checkmail == 0) {
        mvprintw(LINES / 2 + 2, COLS / 2, "Wrong format of email!");
        free(karbar->name);
        free(karbar->password);
        free(karbar->email);
        getch(); 
        endwin();
        return;
    }

    file = fopen(filename, "w"); 
    fprintf(file, "Name: %s\nPassword: %s\nEmail: %s\n", karbar->name, karbar->password, karbar->email);
    fclose(file);
    clear(); 
    mvprintw(LINES / 2, COLS / 2, "Account created successfully."); 
    refresh(); 

    getch(); 
    endwin();

    clear();
}


void login_account(player* karbar) {
    clear();

    karbar->name = (char*)malloc(MAX_SIZE_char * sizeof(char));
    karbar->password = (char*)malloc(MAX_SIZE_char * sizeof(char));
    karbar->email = (char*)malloc(MAX_SIZE_char * sizeof(char));
    char inputName[MAX_SIZE_char];
    char inputPass[MAX_SIZE_char];

    initscr();
    mvprintw(LINES / 2, COLS / 2, "Enter your name: ");
    getnstr(inputName, MAX_SIZE_char);
    snprintf(filename, MAX_SIZE_char, "%s.txt", inputName);

    file = fopen(filename, "r");
    if(file == NULL) {
        clear(); 
        mvprintw(LINES / 2, COLS / 2, "Account does not exist.");
        refresh();
        getch();
        endwin();
        free(karbar->name);
        free(karbar->password);
        free(karbar->email);
        return;
    }

    fscanf(file, "Name: %s\nPassword: %s\n", karbar->name, karbar->password); 
    fclose(file);

    mvprintw(LINES / 2 + 1, COLS / 2, "Enter your password: "); 
    getnstr(inputPass, MAX_SIZE_char);

    if(strcmp(karbar->password, inputPass) == 0) {
        mvprintw(LINES / 2 + 2, COLS / 2, "Login succesful."); 
        refresh();
        getch();
        pregame_menu(karbar);
    }
    else {
        mvprintw(LINES / 2 + 2, COLS / 2, "Wrong password.");
        refresh();
        getch();
        endwin();
        free(karbar->name);
        free(karbar->password);
        free(karbar->email);
    }
    
}


void scoreboard(player* karbar, int gold_num) {
    file = fopen(filename, "r");
    //karbar->gold = 0;
    //fprintf(file, "points: %d", (int)karbar->gold);
    mvprintw(0, 10, "Your points: %d", karbar->gold);
    mvprintw(1, 10, "Recent points: %d", 5);
    getch();
}

void login_menu(player* karbar) {
    char inputPass [MAX_SIZE_char];
    int choice;

    initscr();
    clear();
    mvprintw(LINES / 2 - 1, COLS / 2, "1. Sign Up");
    mvprintw(LINES / 2 , COLS / 2, "2. Login");
    mvprintw(LINES / 2 + 1, COLS / 2, "3. Login as guest");
    mvprintw(LINES / 2 + 2, COLS / 2, "4. Forgot password");
    mvprintw(LINES / 2 + 3, COLS / 2, "Enter your choice: ");

    scanw("%d", &choice);

    switch (choice) {
        case 1:
            create_account(karbar);
            break;
        case 2:
            login_account(karbar);
            break;
        case 3:
            pregame_menu(karbar);
            break;
        case 4:
            mvprintw(LINES / 2 + 1, COLS / 2, "Enter your new password: "); 
            getnstr(inputPass, MAX_SIZE_char);
            refresh();
            pregame_menu(karbar);
            break;
        default:
            refresh(); 
            getch(); 
            endwin(); 
            break;
    }

    endwin();
}


void pregame_menu(player* karbar) {
    clear();
    int choice;

    mvprintw(LINES / 2 - 1, COLS / 2, "1. New Game"); 
    mvprintw(LINES / 2, COLS / 2, "2. Previous Game"); 
    mvprintw(LINES / 2 + 1, COLS / 2, "3. Settings"); 
    mvprintw(LINES / 2 + 2, COLS / 2, "4. Scoreboard"); 
    mvprintw(LINES / 2 + 3, COLS / 2, "Enter your choice: ");

    scanw("%d", &choice);

    switch (choice) {
        case 1:
            clear();
            start_game();
            break;
        case 2:
            clear();
            start_game();
            break;
        case 3:
            settings(karbar);
            break;
        case 4:
            scoreboard(karbar, 0);
            break;
    }
}

void settings(player* karbar) {
    clear();
    int choice;

    mvprintw(LINES / 2 - 1, COLS / 2, "1. Select difficulity"); 
    mvprintw(LINES / 2, COLS / 2, "2. Select color"); 
    mvprintw(LINES / 2 + 1, COLS / 2, "Enter your choice: ");
    scanw("%d", &choice);

    switch (choice) {
        int i;
        int j;
        case 1:
            clear();
            mvprintw(LINES / 2 - 1, COLS / 2, "1. Easy"); 
            mvprintw(LINES / 2, COLS / 2, "2. Medium"); 
            mvprintw(LINES / 2 + 1, COLS / 2, "3. Hard");
            mvprintw(LINES / 2 + 2, COLS / 2, "Enter your choice: ");
            scanw("%d", &i);
            switch(i) {
                case 1:
                    dif_num = 1;
                    break;
                case 2:
                    dif_num = 2;
                case 3:
                    dif_num = 3;
            }
            break;
        case 2:
            clear();    
            mvprintw(LINES / 2 - 1, COLS / 2, "Select a color:"); 
            mvprintw(LINES / 2, COLS / 2, "1. Red"); 
            mvprintw(LINES / 2 + 1, COLS / 2, "2. Green"); 
            mvprintw(LINES / 2 + 2, COLS / 2, "3. Blue"); 
            mvprintw(LINES / 2 + 3, COLS / 2, "Enter your choice: ");
            scanw("%d", &j);
            switch(j) {
                case 1: 
                    player_color = 1;
                    break; 
                case 2: 
                    player_color = 2;
                    break; 
                case 3: 
                    player_color = 3; 
                    break;
            }
            break;
    }
    refresh();
    pregame_menu(karbar);
}


void player_movement(Monster *monster, Position* newPos, player* karbar, char** layout) {
    

    if(layout[karbar->pos.y][karbar->pos.x] == '$') {
        karbar->gold += 5;
        layout[karbar->pos.y][karbar->pos.x] = '.';
        
        mvprintw(0, 0, "Gold: %d    Hunger: %d", karbar->gold, karbar->hunger);   
    }
    if(layout[karbar->pos.y][karbar->pos.x] == '*') {
        karbar->hunger += 5;
        layout[karbar->pos.y][karbar->pos.x] = '.';
        
        mvprintw(0, 0, "Gold: %d    Hunger: %d", karbar->gold, karbar->hunger);   
    }
    if(layout[karbar->pos.y][karbar->pos.x] == 's' || layout[karbar->pos.y][karbar->pos.x] == 'a' ||layout[karbar->pos.y][karbar->pos.x] == 'b') {
        char ch;
        
        layout[karbar->pos.y][karbar->pos.x] = '.';
        
        pickup_sword(karbar, karbar->pos);
        mvprintw(0, 0, "Gold: %d    Hunger: %d", karbar->gold, karbar->hunger); 
         
        layout[karbar->pos.y][karbar->pos.x] = '.';
    }
    if(layout[karbar->pos.y][karbar->pos.x] == 'S' ||
       layout[karbar->pos.y][karbar->pos.x] == 'F' ||
       layout[karbar->pos.y][karbar->pos.x] == 'G' ||
       layout[karbar->pos.y][karbar->pos.x] == 'D' ||
       layout[karbar->pos.y][karbar->pos.x] == 'U') {
        combat(karbar, monster);
    }
    mvprintw(karbar->pos.y, karbar->pos.x, "%c", layout[karbar->pos.y][karbar->pos.x]);
 
    karbar->pos.y = newPos->y;
    karbar->pos.x = newPos->x;
    attron(player_color);
    mvaddch(karbar->pos.y, karbar->pos.x, '@');
    //mvprintw(karbar->pos.y, karbar->pos.x, "@");
    attroff(player_color);
    move(karbar->pos.y, karbar->pos.x);
    
}


Position* handle_movement(int input, player* karbar) {
    Position* newPos = malloc(sizeof(Position));
    if(newPos == NULL) {
        return NULL;
    }
    switch (input) {
        case 'w':
            newPos->y = karbar->pos.y - 1;
            newPos->x = karbar->pos.x;
            break;
        case 'e':
            newPos->y = karbar->pos.y - 1;
            newPos->x = karbar->pos.x + 1;
            break;
        case 's':
            newPos->y = karbar->pos.y + 1;
            newPos->x = karbar->pos.x;
            break;
        case 'x':
            newPos->y = karbar->pos.y + 1;
            newPos->x = karbar->pos.x + 1;
            break;
        case 'a':
            newPos->y = karbar->pos.y;
            newPos->x = karbar->pos.x - 1;
            break;
        case 'q':
            newPos->y = karbar->pos.y - 1;
            newPos->x = karbar->pos.x - 1;
            break;
        case 'd':
            newPos->y = karbar->pos.y;
            newPos->x = karbar->pos.x + 1;
            break;
        case 'z':
            newPos->y = karbar->pos.y + 1;
            newPos->x = karbar->pos.x - 1;
    }

    return newPos;
}


void check_pos(Position* newPos, player* hero, char** layout, Monster* monster) {
    int ch = mvwinch(stdscr, newPos->y, newPos->x) & A_CHARTEXT;
    switch(ch) {
        case '.':
        case '+':
        case '#':
        case '$':
        case '*':
        case 's':
        case 'S':
        case 'F':
        case 'G':
        case 'D':
        case 'a':
        case 'b':
            player_movement(monster, newPos, hero, layout);
            break;
        default:
            move(hero->pos.y, hero->pos.x);
            break;
    }
}


void place_player(player* hero, Room room) {
    int y, x;

    y = rand() % (room.height - 2) + room.pos.y + 1;
    x = rand() % (room.width - 2) + room.pos.x + 1;

    hero->pos.y = y;
    hero->pos.x = x;
}


Room creare_room(int max_height, int max_width) {
    Room room;
    room.door = malloc(4 * sizeof(Position));
    int c = rand() % 4;
    int k = rand() % 5 + 1;
    room.gold = malloc(c * sizeof(Position));
    room.food = malloc(k * sizeof(Position));

    room.height = (rand() % (max_height - ROOM_MIN_SIZE + 1)) + ROOM_MIN_SIZE;
    room.width = (rand() % (max_width - ROOM_MIN_SIZE + 1)) + ROOM_MIN_SIZE;
    room.pos.y = rand() % (LINES - room.height + 1) + 1;
    room.pos.x = rand() % (COLS - room.width + 1);
    room.layout = malloc(room.height * sizeof(char*));

    for(int i = 0; i < room.height; i++) {
        room.layout[i] = malloc(room.width * sizeof(char));

        for(int j = 0; j < room.width; j++) {
            if(i == 0 || i == room.height - 1) {
                room.layout[i][j] = '_';
            }
            else if(j == 0 || j == room.width - 1) {
                room.layout[i][j] = '|';
            }
            else {
                room.layout[i][j] = '.';
            }
        }
    }

    room.door[0].y = 0; 
    room.door[0].x = rand() % room.width;
    room.door[1].y = room.height - 1; 
    room.door[1].x = rand() % room.width;
    room.door[2].y = rand() % room.height; 
    room.door[2].x = 0;
    room.door[3].y = rand() % room.height; 
    room.door[3].x = room.width - 1;

    for(int i = 0; i < 4; i++) {
        room.layout[room.door[i].y][room.door[i].x] = '+';
    }

    for(int i = 0; i < c; i++) {
        room.gold[i].y = rand() % (room.height - 2) + 1;
        room.gold[i].x = rand() % (room.width - 2) + 1;
    }

    for(int i = 0; i < c; i++) {
        room.layout[room.gold[i].y][room.gold[i].x] = '$';
    }

    for(int i = 0; i < k; i++) {
        room.food[i].y = rand() % (room.height - 2) + 1;
        room.food[i].x = rand() % (room.width - 2) + 1;
    }

    for(int i = 0; i < k; i++) {
        room.layout[room.food[i].y][room.food[i].x] = '*';
    }

    room.layout[rand() % (room.height - 2) + 1][rand() % (room.width - 2) + 1] = 'O';
    room.layout[rand() % (room.height - 2) + 1][rand() % (room.width - 2) + 1] = 'a';
    room.layout[rand() % (room.height - 2) + 1][rand() % (room.width - 2) + 1] = 'b';
    
    return room;
}


void create_rooms(Room rooms[], int num_rooms) {
    int room_count = 0;

    while(room_count < num_rooms) {
        Room newroom = creare_room(LINES, COLS);
        bool overlap = false;

        for(int i = 0; i < room_count; i++) {
            if(check_overlap(newroom, rooms[i])) {
                overlap = true;
                break;
            }
        }

        if(!overlap) {
            rooms[room_count++] = newroom;
        }
    }

}


bool check_overlap(Room room1, Room room2) {
    if(room1.pos.y + room1.height + 10 < room2.pos.y || room2.pos.y + room2.height + 10 < room1.pos.y ||
       room1.pos.x + room1.width + 10 < room2.pos.x  || room2.pos.x + room2.width + 10 < room1.pos.x) {
        return false;
       }
    return true;
}


void print_rooms(Room rooms[MAX_ROOMS], int num_rooms) {
    for (int k = 0; k < num_rooms; k++) {
        for (int i = 0; i < rooms[k].height; i++) {
            for (int j = 0; j < rooms[k].width; j++) {
                mvaddch(rooms[k].pos.y + i, rooms[k].pos.x + j, rooms[k].layout[i][j]);
                if (rooms[k].layout[i][j] == '$') {
                    attron(COLOR_PAIR(COLOR_YELLOW));
                    mvaddch(rooms[k].pos.y + i, rooms[k].pos.x + j, '$');
                    attroff(COLOR_PAIR(COLOR_YELLOW));
                }
                // if(rooms[k].layout[i][j] == '@') {
                //     init_pair(player_color, player_color, COLOR_BLACK);
                //     attron(COLOR_PAIR(player_color));
                //     mvaddch(rooms[k].pos.y + i, rooms[k].pos.x + j, '$');
                //     attroff(COLOR_PAIR(player_color));
                // }
            }
        }
    }
}


char** savePos(Room room[MAX_ROOMS]) {
    int y, x;
    char **Pos;
    Pos = malloc(LINES * sizeof(char*));
    for(y = 0; y < LINES; y++) {
        Pos[y] = malloc(COLS * sizeof(char));
        for(int x = 0; x < COLS; x++) {
            Pos[y][x] = mvinch(y, x);
        }
    }
    return Pos;
}


Room create_treasure_room() {
    Room treasureRoom;
    treasureRoom.height = 10;
    treasureRoom.width = 20;
    treasureRoom.pos.y = rand() % (LINES - treasureRoom.height);
    treasureRoom.pos.x = rand() % (COLS - treasureRoom.width);

    treasureRoom.layout = malloc(treasureRoom.height * sizeof(char*));
    for (int i = 0; i < treasureRoom.height; i++) {
        treasureRoom.layout[i] = malloc(treasureRoom.width * sizeof(char));
        for (int j = 0; j < treasureRoom.width; j++) {
            if (i == 0 || i == treasureRoom.height - 1) {
                treasureRoom.layout[i][j] = '_';
            } else if (j == 0 || j == treasureRoom.width - 1) {
                treasureRoom.layout[i][j] = '|';
            } else {
                treasureRoom.layout[i][j] = '.';
            }
        }
    }
    
    
    for (int i = 0; i < 10; i++) {
        int gold_y = rand() % (treasureRoom.height - 2) + 1;
        int gold_x = rand() % (treasureRoom.width - 2) + 1;
        treasureRoom.layout[gold_y][gold_x] = '$';
    }

    
    int boss_y = rand() % (treasureRoom.height - 2) + 1;
    int boss_x = rand() % (treasureRoom.width - 2) + 1;
    treasureRoom.layout[boss_y][boss_x] = 'B';

    return treasureRoom;
}

Monster create_monster(char symbol, int health, int damage, Position pos) {
    Monster monster;
    monster.symbol = symbol;
    monster.health = health;
    monster.damage = damage;
    monster.pos = pos;
    return monster;
}

void place_monster(Room* room, Monster* monster) {
    room->layout[monster->pos.y][monster->pos.x] = monster->symbol;
}

void create_monsters(Room* rooms, int num_rooms) {
    for (int i = 0; i < num_rooms; i++) {
        Room* room = &rooms[i];

        // Randomly place a monster in each room, but the treasure room gets the boss
        // if (rand() % 5 == 0) {
        //     *room = create_treasure_room();
        //     Position boss_pos = {rand() % (room->height - 2) + 1, rand() % (room->width - 2) + 1};
        //     Monster boss = create_monster('U', 30, 5, boss_pos);
        //     place_monster(room, &boss);
        // } 

            int monster_type = rand() % 5;
            char symbol;
            int health;
            int damage;

            switch (monster_type) {
                case 0:
                    symbol = 'D'; health = 5; damage = 1; break; // Deamon
                case 1:
                    symbol = 'F'; health = 10; damage = 3; break; // Fire monster
                case 2:
                    symbol = 'S'; health = 20; damage = 5; break; // Snake
                case 3:
                    symbol = 'U'; health = 30; damage = 5; break;
                default:
                    symbol = 'G'; health = 15; damage = 4; break; // Giant
            }

            Position monster_pos = {rand() % (room->height - 2) + 1, rand() % (room->width - 2) + 1};
            Monster monster = create_monster(symbol, health, damage, monster_pos);
            place_monster(room, &monster);
    }
}


void combat(player* hero, Monster* monster) {
    mvprintw(0, 49, "Combat begins! You encounter a %c", monster->symbol);
    int player_turn = 1;

 
    while(hero->health > 0 && monster->health > 0) {
        if (player_turn) {
            
            if (hero->sword_equipped) {
                monster->health -= hero->damage; // Player attacks with sword
                mvprintw(0, 49, "You attack the monster! It has %d health left.", monster->health);
            } else {
                mvprintw(0, 49, "You don't have a sword! You cannot attack.");
            }
        } else {
            
            hero->health -= monster->damage;
            mvprintw(0, 49, "The monster attacks! You have %d health left.", hero->health);
        }

        mvprintw(0, 0, "Player Health: %d, Monster Health: %d", hero->health, monster->health);
        refresh();
        player_turn = !player_turn;
        usleep(1000000);
    }

    
    if (hero->health <= 0) {
        mvprintw(0, 0, "You have been defeated by the monster.");
    } else if (monster->health <= 0) {
        mvprintw(0, 0, "You defeated the monster!");
    }
}

void pickup_sword(player* hero, Position sword_pos) {
    if (hero->pos.x == sword_pos.x && hero->pos.y == sword_pos.y) {
        hero->sword_equipped = 1; // Equip the sword
        hero->damage = 10;
    }
}

Position place_sword(Room* room) {
    Position sword_pos;
    sword_pos.y = rand() % (room->height - 2) + 1;
    sword_pos.x = rand() % (room->width - 2) + 1;
    room->layout[sword_pos.y][sword_pos.x] = 's';
    return sword_pos;
}


void start_game() {
    initscr();
    noecho();
    cbreak();
    start_color();
    setlocale(LC_ALL, "");

    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    Room *rooms = malloc(MAX_ROOMS * sizeof(Room));
    rooms->layout = malloc(rooms->height * sizeof(char*));
    for(int i = 0; i < rooms->height; i++) {
        rooms->layout[i] = malloc(rooms->width * sizeof(char));
    }

    for(int i = 1; i < LINES; i++) {
        for(int j = 0; j < COLS; j++) {
            mvaddch(i, j, '#');
        }
    }

    create_rooms(rooms, MAX_ROOMS);
    create_monsters(rooms, MAX_ROOMS);
    place_sword(rooms);
    
    
    print_rooms(rooms, MAX_ROOMS);
    char **layout = savePos(rooms);
    

    player *hero = malloc(sizeof(player));
    Position *newPos = malloc(sizeof(Position));
    Monster *monster = malloc(sizeof(Monster));
    hero->pos.y = newPos->y;
    hero->pos.x = newPos->x;
    place_player(hero, rooms[rand() % MAX_ROOMS]);
    hero->gold = 0;
    hero->damage = 0;
    switch(dif_num) {
        case 1:
            hero->hunger = 50;
            hero->health = 10;
            break;
        case 2:
        hero->hunger = 30;
        hero->health = 10;
            break;
        default:
            hero->hunger = 10;
            hero->health = 20;
            break;
    }

    move(hero->pos.y, hero->pos.x);
    mvprintw(0, 0, "Gold: %d    Hunger: %d  Health: %d  damage: %d", hero->gold, hero->hunger, hero->health, hero->damage);
    
    
    int ch;
    int count_step = 0;
    while ((ch = getch()) != 'Q') {
        newPos = handle_movement(ch, hero);
        if (newPos != NULL) {
            check_pos(newPos, hero, layout, monster);
            free(newPos);
        }
        count_step ++;
        if(count_step > 10) {
            count_step = 0;
            hero->hunger--;
            mvprintw(0, 0, "Gold: %d    Hunger: %d  Health: %d  damage: %d", hero->gold, hero->hunger, hero->health, hero->damage);
        }
        if(hero->hunger <= 0) {
            hero->health --;
        }
        if(hero->health == 0) {
            clear();
            mvprintw(LINES / 2, COLS / 2, "You lost");
            //scoreboard(hero, hero->gold);
            getch();
        }
        if(hero->gold == 30) {
            clear();
            Room t = create_treasure_room();
            print_rooms(&t, 1);
            place_player(hero, t);
            // mvprintw(LINES / 2, COLS / 2, "You won");
            // scoreboard(hero, 30);
            // getch();
        }
        refresh();
    }

    endwin();

    for(int i = 0; i < rooms->height; i++) {
        free(layout[i]);
    }
    
    free(layout);
    free(hero);
    free(rooms->gold);
    free(rooms->food);
    free(rooms);
    
}


int main() {
    srand(time(NULL));
    player* karbar = malloc(sizeof(player));
    karbar->color = 1;
    mvprintw(LINES / 2, COLS / 2, "Halim ra bayad ba shekar khord");
    mvprintw(LINES / 2 + 5, COLS / 2 + 3, "Sun Tzu");
    clear();
    login_menu(karbar);
    
    
    attron(COLOR_PAIR(karbar->color)); 
    mvprintw(karbar->pos.y, karbar->pos.x, "@"); 
    attroff(COLOR_PAIR(karbar->color));

    Room rooms[MAX_ROOMS];

    
    free(karbar->name);
    free(karbar->password);
    free(karbar->email);

    free(karbar);
    
    return 0;
}
