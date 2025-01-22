/* 
Salih Yıldız - 22050111003
Cemilhan Sağlam - 21050111013
Nurgül Yalman - 21050111072
Ege Ündeniş - 22050111049
Dilan Aslan - 23050121004 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>

#define GRID_SIZE 8
#define EMPTY '.'
#define SHIP 'S'
#define HIT 'X'
#define MISS 'O'
#define BATTLESHIP_SIZE 4
#define CRUISER_SIZE 3
#define DESTROYER_SIZE 2
#define NUM_BATTLESHIPS 1
#define NUM_CRUISERS 2
#define NUM_DESTROYERS 2

// structure for the grid and ship placement
typedef struct
{
    char grid[GRID_SIZE][GRID_SIZE];
    int hit_positions[GRID_SIZE * GRID_SIZE];
    int attack_queue[GRID_SIZE * GRID_SIZE][2];
    int queue_start;
    int queue_end;
} Player;

void init_ncurses();
void end_ncurses();
void init_grid(Player *player);
void display_grid(char grid[GRID_SIZE][GRID_SIZE], int start_y, int start_x);
void place_ships(Player *player);
int place_single_ship(Player *player, int ship_size);
int is_valid_position(Player *player, int x, int y, int ship_size, int horizontal);
void menu(Player *player1, Player *player2);
void draw_menu(WINDOW *menu_win, int highlight, char **choices, int n_choices);
void show_grids(Player *player1, Player *player2);
void save_game(Player *player1, Player *player2);
void load_game(Player *player1, Player *player2);
int attack(Player *attacker, Player *defender, int x, int y);
int check_win(Player *player);
void auto_game_loop(Player *player1, Player *player2);
void smart_attack(Player *attacker, int *x_ptr, int *y_ptr);

int main()
{
    init_ncurses();

    Player player1, player2;
    init_grid(&player1);
    init_grid(&player2);
    place_ships(&player1);
    place_ships(&player2);
    menu(&player1, &player2);
    end_ncurses();
    return 0;
}

// initializes ncurses
void init_ncurses()
{
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    srand(time(NULL));
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
}

// ends ncurses
void end_ncurses()
{
    endwin();
}

// saves the game
void save_game(Player *player1, Player *player2)
{
    FILE *file = fopen("game.txt", "w");
    if (file == NULL)
    {
        perror("Failed to open file for saving");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            fprintf(file, "%c", player1->grid[i][j]);
        }
    }

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            fprintf(file, "%c", player2->grid[i][j]);
        }
    }

    fclose(file);
}

// loads the game
void load_game(Player *player1, Player *player2)
{
    FILE *file = fopen("game.txt", "r");
    if (file == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (fscanf(file, " %c", &player1->grid[i][j]) != 1)
            {
                fprintf(stderr, "Error reading Player 1's grid\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (fscanf(file, " %c", &player2->grid[i][j]) != 1)
            {
                fprintf(stderr, "Error reading Player 2's grid\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}

// ,nitializes the grid
void init_grid(Player *player)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            player->grid[i][j] = EMPTY;
        }
    }
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        player->hit_positions[i] = 0;
    }
    player->queue_start = 0;
    player->queue_end = 0;
}

// displays the grid
void display_grid(char grid[GRID_SIZE][GRID_SIZE], int start_y, int start_x)
{
    for (int i = 0; i <= GRID_SIZE; i++)
    {
        mvprintw(start_y + i, start_x - 2, "|");
        mvprintw(start_y + i, start_x + GRID_SIZE * 2, "|");
    }
    for (int j = 0; j <= GRID_SIZE; j++)
    {
        mvprintw(start_y - 1, start_x + j * 2, "--");
        mvprintw(start_y + GRID_SIZE, start_x + j * 2, "--");
    }

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (grid[i][j] == SHIP)
            {
                attron(COLOR_PAIR(2));
            }
            else if (grid[i][j] == HIT)
            {
                attron(COLOR_PAIR(3));
            }
            else if (grid[i][j] == MISS)
            {
                attron(COLOR_PAIR(4));
            }
            else
            {
                attron(COLOR_PAIR(1));
            }

            mvprintw(start_y + i, start_x + j * 2, "%c ", grid[i][j]);
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4));
        }
    }
}

// places all the ships
void place_ships(Player *player)
{
    int success;

    success = 0;
    while (!success)
    {
        success = place_single_ship(player, BATTLESHIP_SIZE);
    }

    for (int i = 0; i < NUM_CRUISERS; i++)
    {
        success = 0;
        while (!success)
        {
            success = place_single_ship(player, CRUISER_SIZE);
        }
    }

    for (int i = 0; i < NUM_DESTROYERS; i++)
    {
        success = 0;
        while (!success)
        {
            success = place_single_ship(player, DESTROYER_SIZE);
        }
    }
}

// places a single ship of given size
int place_single_ship(Player *player, int ship_size)
{
    int x, y, horizontal;

    for (int attempt = 0; attempt < 200; attempt++)
    {
        x = rand() % GRID_SIZE;
        y = rand() % GRID_SIZE;
        horizontal = rand() % 2;

        if (is_valid_position(player, x, y, ship_size, horizontal))
        {
            for (int i = 0; i < ship_size; i++)
            {
                if (horizontal)
                {
                    player->grid[x][y + i] = SHIP;
                }
                else
                {
                    player->grid[x + i][y] = SHIP;
                }
            }
            return 1;
        }
    }
    return 0;
}

// checks if a ship can be placed at the specified position
int is_valid_position(Player *player, int x, int y, int ship_size, int horizontal)
{
    for (int i = 0; i < ship_size; i++)
    {
        int nx = x + (horizontal ? 0 : i);
        int ny = y + (horizontal ? i : 0);

        if (nx < 0 || nx >= GRID_SIZE || ny < 0 || ny >= GRID_SIZE)
        {
            return 0;
        }

        if (player->grid[nx][ny] == SHIP)
        {
            return 0;
        }
    }

    for (int i = -1; i <= ship_size; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            int nx = x + (horizontal ? 0 : i) + (horizontal ? j : 0);
            int ny = y + (horizontal ? i : 0) + (horizontal ? 0 : j);

            if (nx < 0 || nx >= GRID_SIZE || ny < 0 || ny >= GRID_SIZE)
            {
                continue;
            }

            if (player->grid[nx][ny] == SHIP)
            {
                return 0;
            }
        }
    }
    return 1;
}

// ai attack!
void smart_attack(Player *attacker, int *x_ptr, int *y_ptr)
{
    int x, y;
    int index;

    if (attacker->queue_start != attacker->queue_end)
    {
        // There are positions in the attack queue
        x = attacker->attack_queue[attacker->queue_start][0];
        y = attacker->attack_queue[attacker->queue_start][1];
        attacker->queue_start++;
        index = x * GRID_SIZE + y;
    }
    else
    {
        while (1)
        {
            x = rand() % GRID_SIZE;
            y = rand() % GRID_SIZE;
            index = x * GRID_SIZE + y;

            if (attacker->hit_positions[index] == 0)
            {
                break;
            }
        }
    }
    attacker->hit_positions[index] = 1;
    *x_ptr = x;
    *y_ptr = y;
}

// normal attack
int attack(Player *attacker, Player *defender, int x, int y)
{
    if (defender->grid[x][y] == SHIP)
    {
        defender->grid[x][y] = HIT;
        return 1;
    }
    else if (defender->grid[x][y] == EMPTY)
    {
        defender->grid[x][y] = MISS;
        return 0;
    }
    return -1; // already hit/missed
}

// checks if a player has won
int check_win(Player *player)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (player->grid[i][j] == SHIP)
            {
                return 0;
            }
        }
    }
    return 1; // if all ships destroyed
}

// manages player turns
void auto_game_loop(Player *player1, Player *player2)
{
    int x, y, result;
    int turn = 1;
    int game_over = 0;

    nodelay(stdscr, TRUE);

    while (!game_over)
    {
        clear();
        show_grids(player1, player2);

        int ch = getch();
        if (ch == 's' || ch == 'S') // 's' key for saving
        {
            save_game(player1, player2);
            mvprintw(27, 0, "Game saved successfully!");
            refresh();
            sleep(1);
        }

        if (turn == 1)
        {
            smart_attack(player1, &x, &y);
            result = attack(player1, player2, x, y);

            if (result == 1)
            {
                int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                for (int i = 0; i < 4; i++)
                {
                    int nx = x + directions[i][0];
                    int ny = y + directions[i][1];
                    int nindex = nx * GRID_SIZE + ny;

                    if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE)
                    {
                        if (player1->hit_positions[nindex] == 0)
                        {
                            player1->attack_queue[player1->queue_end][0] = nx;
                            player1->attack_queue[player1->queue_end][1] = ny;
                            player1->queue_end++;
                        }
                    }
                }
            }

            switch (result)
            {
            case 1:
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(GRID_SIZE + 3, 0, "Player 1 attacks (%d, %d) - Hit!", y, x);
                attroff(COLOR_PAIR(3) | A_BOLD);
                break;
            case 0:
                mvprintw(GRID_SIZE + 3, 0, "Player 1 attacks (%d, %d) - Miss!", y, x);
                break;
            default:
                mvprintw(GRID_SIZE + 3, 0, "Player 1 attacks (%d, %d)", y, x);
                break;
            }

            game_over = check_win(player2);
            turn = 2;
        }
        else
        {
            smart_attack(player2, &x, &y);
            result = attack(player2, player1, x, y);

            if (result == 1)
            {
                int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                for (int i = 0; i < 4; i++)
                {
                    int nx = x + directions[i][0];
                    int ny = y + directions[i][1];
                    int nindex = nx * GRID_SIZE + ny;

                    if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE)
                    {
                        if (player2->hit_positions[nindex] == 0)
                        {
                            player2->attack_queue[player2->queue_end][0] = nx;
                            player2->attack_queue[player2->queue_end][1] = ny;
                            player2->queue_end++;
                        }
                    }
                }
            }

            switch (result)
            {
            case 1:
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(GRID_SIZE + 3, 0, "Player 2 attacks (%d, %d) - Hit!", y, x);
                attroff(COLOR_PAIR(3) | A_BOLD);
                break;
            case 0:
                mvprintw(GRID_SIZE + 3, 0, "Player 2 attacks (%d, %d) - Miss!", y, x);
                break;
            default:
                mvprintw(GRID_SIZE + 3, 0, "Player 2 attacks (%d, %d)", y, x);
                break;
            }

            game_over = check_win(player1);
            if (!game_over)
            { // saves game periodically
                save_game(player1, player2);
            }
            turn = 1;
        }

        if (game_over)
        {
            show_grids(player1, player2);
            attron(COLOR_PAIR(4) | A_BOLD);
            mvprintw(GRID_SIZE + 5, 0, "Game Over! Player %d wins!", turn == 1 ? 2 : 1);
            attroff(COLOR_PAIR(4) | A_BOLD);
            mvprintw(GRID_SIZE + 6, 0, "Press any key to continue...");

            refresh();
            getch();
            init_grid(player1);
            init_grid(player2);
            place_ships(player1);
            place_ships(player2);
            break;
        }

        refresh();
        usleep(250000);
    }
}

// menu
void menu(Player *player1, Player *player2)
{
    char *choices[] = {
        "Start New Game",
        "Display Grids",
        "Re-locate Ships",
        "Save Game",
        "Load Game",
        "Exit"};
    int n_choices = sizeof(choices) / sizeof(char *);
    int highlight = 1;
    int choice = 0;

    WINDOW *menu_win = newwin(10, 40, (LINES - 10) / 2, (COLS - 40) / 2);
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);

    while (1)
    {
        draw_menu(menu_win, highlight, choices, n_choices);

        int c = wgetch(menu_win);
        switch (c)
        {
        case KEY_UP:
            if (highlight == 1)
                highlight = n_choices;
            else
                --highlight;
            break;
        case KEY_DOWN:
            if (highlight == n_choices)
                highlight = 1;
            else
                ++highlight;
            break;
        case 10:
            choice = highlight;
            break;
        default:
            break;
        }

        if (choice != 0)
        {
            break;
        }
    }

    switch (choice)
    {
    case 1: // starts new game
        auto_game_loop(player1, player2);
        break;
    case 2: // displays grids
        show_grids(player1, player2);
        break;
    case 3: // relocates ships
        init_grid(player1);
        init_grid(player2);
        place_ships(player1);
        place_ships(player2);
        show_grids(player1, player2);
        break;
    case 4: // saves the grid
        save_game(player1, player2);
        break;
    case 5: // loads and starts the game
        load_game(player1, player2);
        // show_grids(player1, player2);
        auto_game_loop(player1, player2);
        break;
    case 6: // exits
        delwin(menu_win);
        return;
    default:
        mvprintw(0, 0, "Invalid choice!");
        refresh();
        break;
    }

    delwin(menu_win);
    menu(player1, player2);
}

// draws menu with ncurses
void draw_menu(WINDOW *menu_win, int highlight, char **choices, int n_choices)
{
    int x = 2, y = 2;

    box(menu_win, 0, 0);
    for (int i = 0; i < n_choices; ++i)
    {
        if (highlight == i + 1)
        {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        }
        else
        {
            mvwprintw(menu_win, y, x, "%s", choices[i]);
        }
        ++y;
    }
    
    int menu_y, menu_x;
    getbegyx(menu_win, menu_y, menu_x);
    mvprintw(menu_y + y + 2, menu_x, "The game automatically saves itself after every move.");
    mvprintw(menu_y + y + 3, menu_x, "To save manually, press 'S'.");
    refresh();
    wrefresh(menu_win);


}

// shows grids
void show_grids(Player *player1, Player *player2)
{
    clear();
    mvprintw(0, 0, "Player 1 Grid:");
    display_grid(player1->grid, 2, 0);
    mvprintw(15, 0, "Player 2 Grid:");
    display_grid(player2->grid, 17, 0);
    refresh();
}