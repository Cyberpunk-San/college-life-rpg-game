#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_ATTEMPTS 3
#define TIME_LIMIT 60
#define MAX_TRIES 10
#define MAX_NAME_LENGTH 50
#define MAX_INVENTORY 20
#define MAX_QUESTS 15
#define MAX_LOCATIONS 10
#define MAX_NPCS 20
#define MAX_DESCRIPTION_LENGTH 200
#define MAX_COURSES 5
#define MAX_SKILLS 10
#define MAX_CLUBS 5
#define MAX_EVENTS 10
#define TRANSACTION_EXPENSE 0
#define TRANSACTION_INCOME 1
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50

#define RESET "\033[0m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"


typedef struct
{
    char name[MAX_NAME_LENGTH];
    int health;
    int maxHealth;
    int energy;
    int maxEnergy;
    int exam_prep;
    int charisma;
    int stress;
    int intelligence;
    int experience;
    int level;
    int semester;
    float cgpa;
    int credits;
    int money;
    int dance_skill;
} Character;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    int quantity;
    int value;
    char description[MAX_DESCRIPTION_LENGTH];
} Item;

typedef struct
{
    Item items[MAX_INVENTORY];
    int count;
} Inventory;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    int completed;
    int experienceReward;
    int moneyReward;
    float cgpaBoost;
} Quest;

typedef struct
{
    Quest quests[MAX_QUESTS];
    int count;
} QuestLog;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    int credits;
    float grade;
    int difficulty;
} Course;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    int day;
    int month;
} Event;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    int memberCount;
    int joined;
} Club;

typedef struct
{
    char description[MAX_DESCRIPTION_LENGTH];
    int amount;
    int type; 
    int day;
    int month;
} Transaction;

typedef struct
{
    Transaction transactions[100];
    int count;
} FinancialHistory;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    int unlocked;
    int stress_modifier; 
    int study_bonus;     
} Location;

typedef struct
{
    int act;
    int corruption_exposed;
    int library_secured;
    int dance_battle_won;
    int deanDefeated;
    int dance_skill; 
} Story;

Character player;
Inventory inventory;
QuestLog questLog;
Club clubs[MAX_CLUBS];
int clubCount = 0;
Event events[MAX_EVENTS];
int eventCount = 0;
int currentDay = 1;
int currentMonth = 1;
FinancialHistory financialHistory = {0};
Location locations[MAX_LOCATIONS];
int locationCount = 0;
Story story = {1, 0, 0, 0, 0};
pid_t musicPid = -1;

void signup()
{
    FILE *file = fopen("users.txt", "a+");
    if (file == NULL)
    {
        printf("Error opening file for signup!\n");
        return;
    }
    char username[MAX_USERNAME_LEN], password[MAX_PASSWORD_LEN];
    char file_username[MAX_USERNAME_LEN], file_password[MAX_PASSWORD_LEN];

    int username_exists = 0;
    while (1)
    {
        username_exists = 0;
        printf("Enter username: ");
        fgets(username, MAX_USERNAME_LEN, stdin);
        username[strcspn(username, "\n")] = '\0';
        rewind(file);
        while (fscanf(file, "%s %s", file_username, file_password) != EOF)
        {
            if (strcmp(username, file_username) == 0)
            {
                username_exists = 1;
                break;
            }
        }
        if (username_exists)
        {
            printf("Username already exists! Please try a different username.\n");
        }
        else
        {
            break;
        }
    }
    printf("Enter password: ");
    fgets(password, MAX_PASSWORD_LEN, stdin);
    password[strcspn(password, "\n")] = '\0';
    fprintf(file, "%s %s\n", username, password);
    printf("Signup successful! You can now log in.\n");
    fclose(file);
}

void print_separator(const char *symbol, int length, const char *color)
{
    printf(GREEN "%s", color);
    for (int i = 0; i < length; i++)
    {
        printf(GREEN "%s", symbol);
    }
    printf(GREEN "%s\n", RESET);
}

void saveGame(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("Failed to save the game.");
        return;
    }

    fprintf(file, "Character:\n");
    fprintf(file, "Name:%s\n", player.name);
    fprintf(file, "Health:%d\n", player.health);
    fprintf(file, "Energy:%d\n", player.energy);
    fprintf(file, "Experience:%d\n", player.experience);
    fprintf(file, "Money:%d\n", player.money);

    fprintf(file, "\nInventory:\n");
    for (int i = 0; i < inventory.count; i++)
    {
        fprintf(file, "Item:%s,Quantity:%d,Value:%d\n",
                inventory.items[i].name,
                inventory.items[i].quantity,
                inventory.items[i].value);
    }

    fprintf(file, "\nQuests:\n");
    for (int i = 0; i < questLog.count; i++)
    {
        fprintf(file, "Quest:%s,Completed:%d\n",
                questLog.quests[i].name,
                questLog.quests[i].completed);
    }

    fclose(file);
    printf("Game saved successfully to %s.\n", filename);
}

bool loadGame(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Failed to load the game. File not found.");
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "Name:", 5) == 0)
        {
            sscanf(line, "Name:%49s", player.name);
        }
        else if (strncmp(line, "Health:", 7) == 0)
        {
            sscanf(line, "Health:%d", &player.health);
        }
        else if (strncmp(line, "Energy:", 7) == 0)
        {
            sscanf(line, "Energy:%d", &player.energy);
        }
        else if (strncmp(line, "Experience:", 11) == 0)
        {
            sscanf(line, "Experience:%d", &player.experience);
        }
        else if (strncmp(line, "Money:", 6) == 0)
        {
            sscanf(line, "Money:%d", &player.money);
        }
    }

    fclose(file);
    printf("Game loaded successfully from %s.\n", filename);
    return true;
}

void startBackgroundMusic(const char *musicFile) {
    if (musicPid != -1) {
        return; 
    }

    pid_t pid = fork(); 
    if (pid == 0) {
        
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        
        setsid();

        
        execlp("mpg123", "mpg123", "--quiet", "--loop", "-1", musicFile, NULL);
        perror("Failed to start music"); 
        exit(1);
    } else if (pid > 0) {
        
        musicPid = pid;
    } else {
        perror("Failed to fork process");
    }
}


void stopBackgroundMusic() {
    if (musicPid != -1) {
        
        kill(musicPid, SIGKILL); 
        musicPid = -1;
    }
}


int login()
{
    FILE *file = fopen("users.txt", "r");
    if (file == NULL)
    {
        printf("Error opening file for login!\n");
        return 0; 
    }

    char username[MAX_USERNAME_LEN], password[MAX_PASSWORD_LEN];
    char file_username[MAX_USERNAME_LEN], file_password[MAX_USERNAME_LEN];
    int found = 0;

    printf("Enter username: ");
    fgets(username, MAX_USERNAME_LEN, stdin);
    username[strcspn(username, "\n")] = '\0'; 

    printf("Enter password: ");
    fgets(password, MAX_PASSWORD_LEN, stdin);
    password[strcspn(password, "\n")] = '\0'; 

    while (fscanf(file, "%s %s", file_username, file_password) != EOF)
    {
        if (strcmp(username, file_username) == 0 && strcmp(password, file_password) == 0)
        {
            found = 1;
            break;
        }
    }

    fclose(file);

    if (found)
    {
        printf("Login successful!\n");
        return 1; 
    }
    else
    {
        printf("Invalid username or password.\n");
        return 0; 
    }
}

void login_signup()
{
    int choice;
    while (1)
    {
        printf(GREEN"\n=== Login and Signup System ===\n"RESET);
        printf("1. Signup\n");
        printf("2. Login\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); 

        if (choice == 1)
        {
            signup(); 
        }
        else if (choice == 2)
        {
            if (login())
            { 
                break;
            }
        }
        else
        {
            printf("Invalid choice! Please try again.\n");
        }
    }
}

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void attendWorkshop()
{
    if (player.energy < 40)
    {
        printf("You're too tired to attend a workshop!\n");
        return;
    }

    printf(GREEN"\n----- Available Workshops -----\n"RESET);
    printf("1. KPH (Intelligence +5)\n");
    printf("2. OSDC (Charisma +5)\n");
    printf("3. uCR (Experience +30)\n");

    char input[10];
    printf("Enter workshop choice (0 to cancel): ");
    fgets(input, sizeof(input), stdin);
    int choice = atoi(input);

    switch (choice)
    {
    case 0:
        return;
    case 1:
        player.intelligence += 5;
        player.energy -= 40;
        player.experience += 20;
        printf("You learned new technical skills!\n");

        break;
    case 2:
        player.charisma += 5;
        player.energy -= 35;
        player.experience += 15;
        printf("Your communication skills improved!\n");

        break;
    case 3:
        player.experience += 30;
        player.energy -= 45;
        printf("You gained valuable career insights!\n");

        break;
    default:
        printf("Invalid workshop choice!\n");
    }
}

void printascii(const char *filename)
{
    FILE *filepointer;
    filepointer = fopen(filename, "r");
    char ch;
    while ((ch = fgetc(filepointer)) != EOF)
    {
        printf("%c", ch);
    }
    fclose(filepointer);
}

void TestExam()
{
    printf("HERE OU THE RULES! YOU CAN GAIN 10 CGPA , BUT YOU NEED TO GIVE A TEST\n");
    printf("it's a memory game, the number of rounds you will pass , it will gain that number of cgpa\n");
    printf("In every other round , numbers of previous round are also also asked\n");
    printf("LET'S START!\n");
    int d = 0, i;
    int a[10] = {0};
    srand(time(0));
    while (d != 11)
    {
        d++;
        int l = 1, u = 100;
        int random_number = (rand() % (u - l + 1)) + l;
        a[d - 1] = random_number;
        printf("Remember this number now!-%d", a[d - 1]);
        int b[d];
        sleep(3);
        system("cls");
        int t = 1;
        for (i = 0; i < d; i++)
        {
            scanf("%d", &b[i]);
            if (b[i] != a[i])
            {
                t = 0;
            }
        }
        if (t == 0)
        {
            break;
        }
        else
        {
            player.cgpa++;
        }
    }
}

void checkForEvents()
{
    for (int i = 0; i < eventCount; i++)
    {
        if (events[i].day == currentDay && events[i].month == currentMonth)
        {
            printf("\nEvent Alert: %s\n", events[i].name);
            printf("Description: %s\n", events[i].description);
        }
    }
}
int combat(const char *opponent, int type)
{
    printf("\n=== Battle with %s ===\n", opponent);

    int playerScore = 0;
    int opponentScore = 0;

    switch (type)
    {
    case 0: 
        playerScore = player.health + player.energy + (rand() % 20);
        opponentScore = 100 + (rand() % 20);
        sleep(3);
        break;

    case 1: 
        sleep(1);                                                              
        playerScore = player.charisma + story.dance_skill + (rand() % 10); 
        sleep(1);
        opponentScore = 60 + (rand() % 10);
        sleep(1);                                
        break;

    case 2: 
        playerScore = player.intelligence + player.charisma + (rand() % 20);
        opponentScore = 120 + (rand() % 20);
        sleep(3);
        break;
    }

    printf("Your score: %d\n", playerScore);
    printf("Opponent score: %d\n", opponentScore);

    if (playerScore > opponentScore)
    {
        printf("You won against %s!\n", opponent);
        player.experience += 50;
        return 1;
    }
    else
    {
        printf("You lost to %s!\n", opponent);
        player.stress += 10;
        return 0;
    }
}

void initClubs()
{
    strcpy(clubs[0].name, "Open Source Development Society");
    strcpy(clubs[0].description, "A club for Development enthusiasts and aspiring programmers.");
    clubs[0].memberCount = 30;
    clubs[0].joined = 0;

    strcpy(clubs[1].name, "Knuth Programming hub");
    strcpy(clubs[1].description, "Improve your DSA and Competetive programming skills");
    clubs[1].memberCount = 50;
    clubs[1].joined = 0;

    strcpy(clubs[2].name, "DebSoc");
    strcpy(clubs[2].description, "Improve your public speaking and argumentation skills.");
    clubs[2].memberCount = 25;
    clubs[2].joined = 0;

    strcpy(clubs[3].name, "Crescendo");
    strcpy(clubs[3].description, "Improve your singing and music skills");
    clubs[3].memberCount = 20;
    clubs[3].joined = 0;

    strcpy(clubs[4].name, "Thespian");
    strcpy(clubs[4].description, "Express yourself through acting and stage production.");
    clubs[4].memberCount = 20;
    clubs[4].joined = 0;

    clubCount = 5;
}


void to_lower(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}
int crescendo()
{
    stopBackgroundMusic("bgm.mp3");
    char *songs[] = {
        "jo tum mere ho",
        "baawra",
        "saawre",
        "co2",
        "ek zindagi meri"};
    char *files[] = {
        "1.mp3",
        "2.mp3",
        "3.mp3",
        "4.mp3",
        "5.mp3"};
    int score = 0;
    char user_input[100];

    for (int i = 0; i < 5; i++)
    {
        
        startBackgroundMusic(files[i]);
        sleep(15);
        stopBackgroundMusic(files[i]);
        printf("Guess the song name: ");
        sleep(2);
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = '\0';
        to_lower(user_input);
        if (strcasecmp(user_input, songs[i]) == 0)
        {
            printf("Correct!\n");
            score++;
        }
        else
        {
            printf("Incorrect! The correct answer is: %s\n", songs[i]);
        }
        sleep(1);
    }
    printf("\nGame Over! Your final score is: %d out of 5\n", score);
    startBackgroundMusic("bgm.mp3");
    return score;
}

int displayQuestion(int questionNumber, char question[], char options[][100], char correctAnswer)
{
    printf("\nQuestion %d: %s\n", questionNumber, question);
    for (int i = 0; i < 4; i++)
    {
        printf("%c. %s\n", 'A' + i, options[i]);
    }

    char userAnswer;
    printf("\nEnter your answer (A/B/C/D): ");
    scanf(" %c", &userAnswer);
    userAnswer = toupper(userAnswer); 

    if (userAnswer == correctAnswer)
    {
        printf("Correct!\n");
        return 1; 
    }
    else
    {
        printf("Wrong! The correct answer was %c.\n", correctAnswer);
        return 0; 
    }
}

int debsoc()
{   stopBackgroundMusic("bgm.mp3");
    startBackgroundMusic("debsoc.mp3");
    char *questions[] = {
        "Who was the first Governor-General of independent India?",
        "The first battle of Panipat (1526) was fought between which two rulers?",
        "Who was the first Chief Justice of India?",
        "What is the name of the first woman to become the President of the Indian National Congress?",
        "The First Law Commission of India was established in which year?",
    };

    char options[5][4][100] = {
        {"Lord Mountbatten", "Rajagopalachari", "Jawaharlal Nehru", "Rajendra Prasad"},
        {"Babur and Ibrahim Lodi", "Akbar and Rana Pratap", "Nadir Shah and Muhammad Shah", "Sher Shah Suri and Humayun"},
        {"Harilal Jekisundas Kania", "M. Patanjali Sastri", "Rajendra Prasad", "B. R. Ambedkar"},
        {"Aruna Asaf Ali", "Sarojini Naidu", "Annie Besant", "Kamaladevi Chattopadhyay"},
        {"1774", "1833", "1857", "1947"},
    };

    char answers[] = {'A', 'A', 'A', 'C', 'B'};

    int totalQuestions = sizeof(questions) / sizeof(questions[0]);
    int totalScore = 0;

    for (int i = 0; i < totalQuestions; i++)
    {
        totalScore += displayQuestion(i + 1, questions[i], options[i], answers[i]);
        sleep(1);
    }

    printf("\nYour total score: %d/%d\n", totalScore, totalQuestions);
    stopBackgroundMusic("debsoc.mp3");
    startBackgroundMusic("bgm.mp3");
    return totalScore;
}

void displayWord(char word[], int guessed[], int wordLength)
{
    for (int i = 0; i < wordLength; i++)
    {
        if (guessed[i] == 1)
        {
            printf("%c ", word[i]);
        }
        else
        {
            printf("_ ");
        }
    }
    printf("\n");
}

int checkGuess(char word[], char guessedLetter, int wordLength)
{
    for (int i = 0; i < wordLength; i++)
    {
        if (word[i] == guessedLetter)
        {
            return 1;
        }
    }
    return 0;
}

float hangman(char word[])
{
    int wordLength = strlen(word);
    int guessed[wordLength];
    for (int i = 0; i < wordLength; i++)
    {
        guessed[i] = 0;
    }

    int tries = 0;
    int guessedCorrectly = 0;
    double score = 5.0;
    while (tries < MAX_TRIES && guessedCorrectly < wordLength)
    {
        char guess;
        int correctGuess = 0;
        displayWord(word, guessed, wordLength);

        printf("Score: %.1f\n", score);
        printf("Enter a letter: ");
        scanf(" %c", &guess);
        guess = tolower(guess);
        if (!isalpha(guess))
        {
            printf("Please enter a valid letter.\n");
            continue;
        }

        int alreadyGuessed = 0;
        for (int i = 0; i < wordLength; i++)
        {
            if (guessed[i] == 1 && word[i] == guess)
            {
                alreadyGuessed = 1;
                break;
            }
        }

        if (alreadyGuessed)
        {
            printf("You've already guessed the letter '%c'. Try another one.\n", guess);
            continue;
        }

        correctGuess = checkGuess(word, guess, wordLength);
        if (correctGuess)
        {
            printf("Good guess! '%c' is in the word.\n", guess);
            for (int i = 0; i < wordLength; i++)
            {
                if (word[i] == guess)
                {
                    guessed[i] = 1;
                    guessedCorrectly++;
                }
            }
        }
        else
        {
            printf("Sorry, '%c' is not in the word.\n", guess);
            tries++;
            score -= 0.5;
            printf("You have %d more tries. Score: %.1f\n", MAX_TRIES - tries, score);
        }
    }

    if (guessedCorrectly == wordLength)
    {
        printf("\nCongratulations! You've guessed the word: %s\n", word);
        printf("Your final score is: %.1f\n", score);
    }
    else
    {
        printf("\nGame Over! You've run out of tries. The word was: %s\n", word);
        printf("Your final score is: %.1f\n", score);
    }
    return score;
}
float osdc()
{   stopBackgroundMusic("bgm.mp3");
    startBackgroundMusic("osdc.mp3");
    char *wordList[] = {
        "osdclub", "computer", "programming", "development", "language",
        "algorithm", "data", "pointer", "array"};
    int numWords = sizeof(wordList) / sizeof(wordList[0]);
    srand(time(0));
    int randomIndex = rand() % numWords;
    char *word = wordList[randomIndex];
    printf("Welcome to OSDC Club, Give an interview!\n");
    int score = hangman(word);
    stopBackgroundMusic("osdc.mp3");
    startBackgroundMusic("bgm.mp3");
    return score;
}

int thespian()
{   stopBackgroundMusic("bgm.mp3");
    startBackgroundMusic("thespian.mp3");
    char answer[50];
    int score = 0;

    printf("GIVE AN INTERVIEW!! TO JOIN THE FULL NAUTANKI i.e. THESPIAN CLUB\n\n");

    printf("1. 🕵♂️🔍💼\n");
    printf("Enter your guess: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = 0;
    for (int i = 0; answer[i]; i++)
    {
        answer[i] = tolower(answer[i]);
    }
    if (strcmp(answer, "detective byomkesh bakshi") == 0)
    {
        printf("Correct!\n");
        score++;
    }
    else
    {
        printf("Wrong! The correct answer is 'Detective Byomkesh Bakshi'.\n");
    }
    sleep(1);
    printf("\n2. 🎤👩‍🎤🎶\n");
    printf("Enter your guess: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = 0;
    for (int i = 0; answer[i]; i++)
    {
        answer[i] = tolower(answer[i]);
    }
    if (strcmp(answer, "rock on") == 0)
    {
        printf("Correct!\n");
        score++;
    }
    else
    {
        printf("Wrong! The correct answer is 'Rock On!!'.\n");
    }
    sleep(1);
    printf("\n3. 🎩🪄✨\n");
    printf("Enter your guess: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = 0;
    for (int i = 0; answer[i]; i++)
    {
        answer[i] = tolower(answer[i]);
    }
    if (strcmp(answer, "baazigar") == 0)
    {
        printf("Correct!\n");
        score++;
    }
    else
    {
        printf("Wrong! The correct answer is 'Baazigar'.\n");
    }
    sleep(1);
    printf("\n4. 🍷💃🎉\n");
    printf("Enter your guess: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = 0;
    for (int i = 0; answer[i]; i++)
    {
        answer[i] = tolower(answer[i]);
    }
    if (strcmp(answer, "zindagi na milegi dobara") == 0)
    {
        printf("Correct!\n");
        score++;
    }
    else
    {
        printf("Wrong! The correct answer is 'Zindagi Na Milegi Dobara'.\n");
    }
    sleep(1);
    printf("\n5. 🦁👑\n");
    printf("Enter your guess: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = 0;
    if (strcmp(answer, "the lion king") == 0)
    {
        printf("Correct!\n");
        score++;
        for (int i = 0; answer[i]; i++)
        {
            answer[i] = tolower(answer[i]);
        }
    }
    else
    {
        printf("Wrong! The correct answer is 'The Lion King'.\n");
    }
    printf("\nYour final score is: %d/5\n", score);
    stopBackgroundMusic("thespian.mp3");
    startBackgroundMusic("bgm.mp3");
    return score;
}

void shuffleArray(const char *array[], const char *answers[], int size)
{
    srand(time(0));
    for (int i = size - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        const char *temp = array[i];
        array[i] = array[j];
        array[j] = temp;
        temp = answers[i];
        answers[i] = answers[j];
        answers[j] = temp;
    }
}

int askPuzzle(int puzzleNumber, const char *puzzle, const char *correctAnswer)
{
    char answer[100];
    printf("\nPuzzle %d: %s\n", puzzleNumber, puzzle);
    printf("Your answer: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = 0;

    if (strcmp(answer, correctAnswer) == 0)
    {
        printf("Correct! You solved the puzzle!\n");
        return 1;
    }
    else
    {
        printf("Oops! Incorrect. The correct answer is: %s\n", correctAnswer);
        return 0;
    }
}

int KPH()
{   stopBackgroundMusic("bgm.mp3");
    startBackgroundMusic("kph.mp3");
    const char *puzzles[] = {
        "I speak without a mouth and hear without ears. I have no body, but I come alive with wind. What am I?",
        "The more you take, the more you leave behind. What am I?",
        "What comes once in a minute, twice in a moment, but never in a thousand years?",
        "I am always hungry, I must always be fed. The finger I touch, will soon turn red. What am I?",
        "What can travel around the world while staying in the corner?"};

    const char *answers[] = {
        "echo",
        "footsteps",
        "m",
        "fire",
        "stamp"};

    int totalPuzzles = sizeof(puzzles) / sizeof(puzzles[0]);

    shuffleArray(puzzles, answers, totalPuzzles);

    printf("Welcome to the Logical Puzzle Game!\n");

    int totalScore = 0;
    for (int i = 0; i < totalPuzzles; i++)
    {
        totalScore += askPuzzle(i + 1, puzzles[i], answers[i]);
        printf("\n");
    }

    printf("\nGame Over! You solved %d out of %d puzzles correctly!\n", totalScore, totalPuzzles);
    stopBackgroundMusic("kph.mp3");
    startBackgroundMusic("bgm.mp3");
    return totalScore;
}

void joinClub()
{
    printf(GREEN"\nAvailable Clubs:\n"RESET);
    for (int i = 0; i < clubCount; i++)
    {
        printf("%d. %s (%d members)\n", i + 1, clubs[i].name, clubs[i].memberCount);
        printf("   Description: %s\n", clubs[i].description);
    }

    int choice;
    char input[10];
    printf("Enter the number of the club you want to join: ");
    fgets(input, sizeof(input), stdin);
    choice = atoi(input);

    if (choice >= 1 && choice <= clubCount)
    {
        float score;
        if (!clubs[choice - 1].joined)
        {
            if (choice == 1)
            {
                score = osdc();
            }
            else if (choice == 2)
            {
                score = KPH();
            }
            else if (choice == 3)
            {
                score = debsoc();
            }
            else if (choice == 4)
            {
                score = crescendo();
            }
            else if (choice == 5)
            {
                score = thespian();
            }
            if (score >= 3.5)
            {
                printf("YOU PASSED THE INTERVIEW!WELCOME TO THE CLUB");
                clubs[choice - 1].joined = 1;
                clubs[choice - 1].memberCount++;
                printf("You've joined the %s!\n", clubs[choice - 1].name);
                sleep(3);
                player.experience += 20;
                player.charisma += 2;
                player.stress += 5;
                if (player.stress > 100)
                {
                    player.stress = 100;
                }
            }
            else
            {
                printf("BETTER LUCK NEXT TIME!");
                sleep(3);
            }
        }

        else
        {
            printf("You're already a member of this club.\n");
        }
    }
    else
    {
        printf("Invalid club choice.\n");
    }
}
void printInventory()
{
    printf(GREEN"\n----- Inventory -----\n"RESET);
    if (inventory.count == 0)
    {
        printf("Your inventory is empty.\n");
    }
    else
    {
        for (int i = 0; i < inventory.count; i++)
        {
            printf("%d. %s x%d (Value: $%d)\n", i + 1, inventory.items[i].name, inventory.items[i].quantity, inventory.items[i].value);
            printf("Description: %s\n", inventory.items[i].description);
        }
    }
}

void cafefight()
{   stopBackgroundMusic("bgm.mp3");
    startBackgroundMusic("hungry.mp3");
    printf(GREEN"\n=== CAFETERIA SHOWDOWN! ===\n"RESET);
    sleep(1);
    printf("Hungry students are fighting for the last plate of food!\n");
    sleep(1);
    printf("Rules:\n");
    sleep(1);
    printf("1. Choose a number between 1 and 10\n");
    sleep(1);
    printf("2. If your number matches theirs, you lose energy\n");
    sleep(1);
    printf("3. If your number is different, you gain energy\n");
    sleep(1);
    printf("4. The battle lasts for 5 rounds\n\n");
    sleep(1);

    int rounds = 5;
    int consecutive_hits = 0;

    for (int round = 1; round <= rounds; round++)
    {
        printf("\nRound %d/%d\n", round, rounds);
        printf("Your current energy: %d\n", player.energy);

        int num;
        char input[10];
        do
        {
            printf("Enter your move (1-10): ");
            fgets(input, sizeof(input), stdin);
            num = atoi(input);
        } while (num < 1 || num > 10);

        int opponent_move = (rand() % 10) + 1;
        printf("Hungry student's move: %d\n", opponent_move);

        if (num == opponent_move)
        {
            player.energy = (player.energy <= 10) ? 0 : player.energy - 10;
            consecutive_hits++;
            printf("Hit! Lost 10 energy\n");

            if (consecutive_hits >= 3)
            {
                player.health = (player.health <= 10) ? 0 : player.health - 10;
                printf("Three consecutive hits! Lost 10 health\n");
            }
        }
        else
        {
            player.energy = (player.energy >= 90) ? 100 : player.energy + 10;
            consecutive_hits = 0;
            printf("Dodge! Gained 10 energy\n");
        }

        if (player.health <= 0)
        {
            printf("\nYou've been defeated! Better luck next time...\n");
            return;
        }

        printf("\nPress Enter for next round...");
        getchar();
    }

    printf("\nYou survived the cafeteria battle!\n");

    player.experience += 20;
    printf("Gained 20 experience points for your bravery!\n");
    stopBackgroundMusic("hungry.mp3");
    startBackgroundMusic("bgm.mp3");
}

void eat()
{
    printf("Where would you like to eat?\n");
    printf("1. Cafeteria (Rs.30)\n");
    printf("2. Local Restaurant (Rs.150)\n");
    printf("3. Mess (Rs.0)\n");

    int choice;
    char input[10];
    printf("Enter your choice: ");
    fgets(input, sizeof(input), stdin);
    choice = atoi(input);
    int a = rand();
    switch (choice)
    {

    case 1:
        if (a % 2 == 0)
        {
            cafefight();
        }
        else
        {
            if (player.money >= 30)
            {
                player.money -= 30;
                player.health += 15;
                player.energy += 20;
                printf("You eat a meal at the cafeteria.\n");
            }
            else
            {
                printf("You don't have enough money to eat at the cafeteria.\n");
            }
        }
        break;
    case 2:
        if (player.money >= 150)
        {
            player.money -= 150;
            player.health += 25;
            player.energy += 30;
            player.stress -= 10;
            printf("You enjoy a delicious meal at a local restaurant.\n");
        }
        else
        {
            printf("You don't have enough money to eat at a restaurant.\n");
        }
        break;
    case 3:
        player.health += 10;
        player.energy += 15;
        printf("You eat from the campus mess, Annapurna.\n");
        break;
    default:
        printf("Invalid choice. You skip the meal.\n");
        player.health -= 5;
        player.energy -= 10;
        player.stress += 5;
    }

    if (player.health > player.maxHealth)
        player.health = player.maxHealth;
    if (player.energy > player.maxEnergy)
        player.energy = player.maxEnergy;
    if (player.stress < 0)
        player.stress = 0;
    if (player.stress > 100)
        player.stress = 100;
}

void initCharacter()
{
    printf("Enter your name: ");
    fgets(player.name, MAX_NAME_LENGTH, stdin);
    player.name[strcspn(player.name, "\n")] = 0;
    player.energy = 100;
    player.maxEnergy = 100;
    player.health = 100;
    player.maxHealth = 100;
    player.exam_prep = 0;
    player.charisma = 10;
    player.intelligence = 10;
    player.stress = 0;
    player.experience = 0;
    player.level = 1;
    player.semester = 1;
    player.cgpa = 0.0;
    player.credits = 0;
    player.money = 1000;
}

void study()
{
    printf("Where do you want to study?\n1. LRC (energy=50, exam prep gain=20, Intelligence gain = 3)\n2. Room (energy=20, exam prep gain=5, Intelligence gain = 2)\n3. With Friends (energy=30, charisma gain=10, exam prep gain=20, Intelligence gain = 3)\n");
    int st;
    scanf("%d", &st);
    getchar();
    printf("\nYou studied successfully!\n");
    switch (st)
    {
    case 1:
        player.energy -= 50;
        player.exam_prep += 20;
        player.intelligence += 3;
        break;
    case 2:
        player.energy -= 20;
        player.exam_prep += 5;
        player.intelligence += 2;
        break;
    case 3:
        player.energy -= 30;
        player.exam_prep += 20;
        player.charisma += 10;
        player.intelligence += 1;
        break;
    default:
        printf("Invalid choice.\n");
    }
}
void fingerExerciseGame()
{   stopBackgroundMusic("bgm.mp3");
    startBackgroundMusic("kph.mp3");
    char *sentences[] = {
        "I said I'm tired, and mom handed me a broom; apparently, sweeping cures fatigue.",
        "Every Indian wedding has three guarantees: dance drama, Paneer ki sabji, and that one aunt interrogating your life.",
        "Parents will proudly tell guests you graduated but forget to mention they chose your career.",
        "I signed up for yoga to find inner peace but found inner back pain instead.",
        "Indian parents call it 'motivation' when they compare you to Sharma ji's genius son."};
    int totalSentences = sizeof(sentences) / sizeof(sentences[0]);
    srand(time(NULL));
    int randomIndex = rand() % totalSentences;
    char *selectedSentence = sentences[randomIndex];
    printf("Welcome to the Finger Exercise Game!\n");
    printf("You need to type the following sentence within %d seconds:\n", TIME_LIMIT);
    printf("'%s'\n\n", selectedSentence);
    char userInput[256];
    time_t start, end;
    int success = 0;
    while (!success)
    {
        printf("Start typing: ");
        start = time(NULL);
        fgets(userInput, sizeof(userInput), stdin);
        userInput[strcspn(userInput, "\n")] = 0;
        end = time(NULL);
        double timeTaken = difftime(end, start);
        if (strcmp(userInput, selectedSentence) == 0 && timeTaken <= TIME_LIMIT)
        {
            printf("\nCongratulations! You typed the sentence correctly in %.2f seconds.\n", timeTaken);
            success = 1;
        }
        else if (timeTaken > TIME_LIMIT)
        {
            printf("\nTime's up! You took %.2f seconds. Try again.\n", timeTaken);
        }
        else
        {
            printf("\nOops! The sentence doesn't match. Try again.\n");
        }
    }
    stopBackgroundMusic("kph.mp3");
    startBackgroundMusic("bgm.mp3");
}

void exercise()
{
    fingerExerciseGame();
    player.energy -= 10;
    player.stress -= 20;
    printf("You exercised! Stress decreased.\n");
}

void strangerTalk()
{
    player.energy -= 10;
    int r = rand() % 10;
    if (r < 5)
    {
        printf("You met a good person!\nCHARISMA INCREASED!\nSTRESS DECREASED!\n");
        player.charisma += 10;
        player.stress = (player.stress >= 10) ? player.stress - 10 : 0;
    }
    else if (r < 8)
    {
        printf("You failed to meet anyone.\n");
    }
    else
    {
        printf("You met a bully! CHARISMA DECREASES!\nSTRESS INCREASED!\n");
        player.stress += 10;
        player.charisma = (player.charisma >= 10) ? player.charisma - 10 : 0;
    }
}

void trackExam()
{
    if (currentDay == 15)
    {
        printf("Your Exam is today!\n");
        TestExam();
    }
    else if (currentDay < 15)
    {
        printf("Your Exam is in %d days.\n", 15 - currentDay);
    }
    else
    {
        printf("Your Exam is in %d days.\n", 45 - currentDay);
    }
}

void mysleep()
{
    player.energy = (player.energy <= 50) ? player.energy + 50 : 100;
    player.health = (player.health + 10 > player.maxHealth) ? player.maxHealth : player.health + 10;
    player.stress = (player.stress >= 10) ? player.stress - 10 : 0;
    currentDay++;
    if (currentDay > 30)
    {
        currentDay = 1;
        currentMonth++;
        if (currentMonth > 12)
        {
            currentMonth = 1;
            player.semester++;
        }
    }
    printf("Sleep success! Energy increased, stress decreased.\n");
    printf("It's now day %d of month %d, semester %d.\n", currentDay, currentMonth, player.semester);
}

void printStatus()
{
    printf(GREEN"\n----- Character Status -----\n"RESET);
    printf("Name: %s\n", player.name);
    printf("Health: %d/%d\n", player.health, player.maxHealth);
    printf("Energy: %d/%d\n", player.energy, player.maxEnergy);
    printf("Intelligence: %d\n", player.intelligence);
    printf("Charisma: %d\n", player.charisma);
    printf("Stress: %d\n", player.stress);
    printf("Experience: %d\n", player.experience);
    printf("Level: %d\n", player.level);
    printf("Semester: %d\n", player.semester);
    printf("CGPA: %.2f\n", player.cgpa);
    printf("Credits: %d\n", player.credits);
    printf("Money: Rs.%d\n", player.money);
    printf("Exam preparation : %d\n", player.exam_prep);
}

void addQuest(const char *questName, const char *questDescription, int expReward)
{
    if (questLog.count < MAX_QUESTS)
    {
        strcpy(questLog.quests[questLog.count].name, questName);
        strcpy(questLog.quests[questLog.count].description, questDescription);
        questLog.quests[questLog.count].completed = 0;
        questLog.quests[questLog.count].experienceReward = expReward;
        questLog.count++;
        printf(GREEN"New quest added: %s\n"RESET, questName);
    }
    else
    {
        printf("Quest log is full!\n");
    }
}

void printQuests()
{
    printf(GREEN"\n----- Quest Log -----\n"RESET);
    if (questLog.count == 0)
    {
        printf("You have no active quests.\n");
    }
    else
    {
        for (int i = 0; i < questLog.count; i++)
        {
            printf("%d. %s: %s\n", i + 1, questLog.quests[i].name, questLog.quests[i].completed ? "Completed" : "In Progress");
            printf("Description: %s\n", questLog.quests[i].description);
        }
    }
}

void manageFinances()
{
    char input[20];
    printf(GREEN"\n=== Financial Management ===\n"RESET);
    printf("Current Balance: ₹%d\n", player.money);

    printf("\n1. View Transaction History");
    printf("\n2. Make a Purchase");
    printf("\n3. Add Income");
    printf("\n4. View Budget Analysis");
    printf("\n0. Back\n");
    printf("\nChoice: ");

    fgets(input, sizeof(input), stdin);
    int choice = atoi(input);

    switch (choice)
    {
    case 1:
        printf("\n=== Transaction History ===\n");
        for (int i = 0; i < financialHistory.count; i++)
        {
            printf("%d/%d: ₹%d (%s) - %s\n",
                   financialHistory.transactions[i].day,
                   financialHistory.transactions[i].month,
                   financialHistory.transactions[i].amount,
                   financialHistory.transactions[i].type ? "Income" : "Expense",
                   financialHistory.transactions[i].description);
        }
        break;

    case 2:
    {
        printf("Enter expense amount: ₹");
        fgets(input, sizeof(input), stdin);
        int amount = atoi(input);

        if (amount > player.money)
        {
            printf("Insufficient funds!\n");
            return;
        }

        printf("Enter description: ");
        char desc[MAX_DESCRIPTION_LENGTH];
        fgets(desc, sizeof(desc), stdin);
        desc[strcspn(desc, "\n")] = 0;

        player.money -= amount;

        Transaction t = {0};
        strcpy(t.description, desc);
        t.amount = amount;
        t.type = TRANSACTION_EXPENSE;
        t.day = currentDay;
        t.month = currentMonth;
        financialHistory.transactions[financialHistory.count++] = t;

        printf("Transaction recorded! New balance: ₹%d\n", player.money);
        break;
    }

    case 3:
    {
        printf("Enter income amount: ₹");
        fgets(input, sizeof(input), stdin);
        int amount = atoi(input);

        printf("Enter source: ");
        char desc[MAX_DESCRIPTION_LENGTH];
        fgets(desc, sizeof(desc), stdin);
        desc[strcspn(desc, "\n")] = 0;

        player.money += amount;

        Transaction t = {0};
        strcpy(t.description, desc);
        t.amount = amount;
        t.type = TRANSACTION_INCOME;
        t.day = currentDay;
        t.month = currentMonth;
        financialHistory.transactions[financialHistory.count++] = t;

        printf("Income recorded! New balance: ₹%d\n", player.money);
        break;
    }
    }
}

void initLocations()
{

    strcpy(locations[locationCount].name, "Main Academic Block");
    strcpy(locations[locationCount].description, "The heart of academic life where Professor Pataka roams. Watch out for surprise quizzes!");
    locations[locationCount].unlocked = 1;
    locations[locationCount].stress_modifier = 10;
    locations[locationCount].study_bonus = 20;
    locationCount++;

    strcpy(locations[locationCount].name, "Chintu's Canteen");
    strcpy(locations[locationCount].description, "The legendary hangout spot. Best maggi in town and source of valuable information!");
    locations[locationCount].unlocked = 1;
    locations[locationCount].stress_modifier = -10;
    locations[locationCount].study_bonus = -5;
    locationCount++;

    strcpy(locations[locationCount].name, "Library");
    strcpy(locations[locationCount].description, "A battleground during exams. Prime study spots are highly contested!");
    locations[locationCount].unlocked = 1;
    locations[locationCount].stress_modifier = 5;
    locations[locationCount].study_bonus = 30;
    locationCount++;

    strcpy(locations[locationCount].name, "Hostel");
    strcpy(locations[locationCount].description, "Your safe haven... usually. Watch out for Risky and Misky!");
    locations[locationCount].unlocked = 1;
    locations[locationCount].stress_modifier = -15;
    locations[locationCount].study_bonus = 10;
    locationCount++;

    strcpy(locations[locationCount].name, "College Ground");
    strcpy(locations[locationCount].description, "Where dance battles and cultural events happen. Perfect for socializing!");
    locations[locationCount].unlocked = 1;
    locations[locationCount].stress_modifier = -20;
    locations[locationCount].study_bonus = -10;
    locationCount++;

    strcpy(locations[locationCount].name, "Computer Lab");
    strcpy(locations[locationCount].description, "Home of coding battles and technical events. Tamboli's favorite spot.");
    locations[locationCount].unlocked = 1;
    locations[locationCount].stress_modifier = 15;
    locations[locationCount].study_bonus = 25;
    locationCount++;

    strcpy(locations[locationCount].name, "Dean's Office");
    strcpy(locations[locationCount].description, "The final boss arena. Currently locked - need to progress story to access.");
    locations[locationCount].unlocked = 0;
    locations[locationCount].stress_modifier = 30;
    locations[locationCount].study_bonus = 0;
    locationCount++;
}

void updateLocationAccess()
{

    if (story.act == 4 && story.corruption_exposed)
    {
        for (int i = 0; i < locationCount; i++)
        {
            if (strcmp(locations[i].name, "Dean's Office") == 0)
            {
                locations[i].unlocked = 1;
                printf("\n=== ALERT: Dean's Office is now accessible! ===\n");
                break;
            }
        }
    }
}

void changeLocation()
{
    updateLocationAccess();

    printf("\n=== Available Locations ===\n");
    int availableLocations = 0;
    for (int i = 0; i < locationCount; i++)
    {
        if (locations[i].unlocked)
        {
            availableLocations++;
            printf("%d. %s\n", availableLocations, locations[i].name);
            printf("   %s\n", locations[i].description);
            if (locations[i].study_bonus != 0)
            {
                printf("   Study Bonus: %+d\n", locations[i].study_bonus);
            }
            if (locations[i].stress_modifier != 0)
            {
                printf("   Stress Impact: %+d\n", locations[i].stress_modifier);
            }
            printf("\n");
        }
    }

    if (availableLocations == 0)
    {
        printf("No locations available!\n");
        return;
    }

    printf("\nChoose location (0 to cancel): ");
    char input[10];
    fgets(input, sizeof(input), stdin);
    int choice = atoi(input) - 1;

    if (choice >= 0 && choice < locationCount && locations[choice].unlocked)
    {

        if (strcmp(locations[choice].name, "Library") == 0 && story.act == 3)
        {   stopBackgroundMusic("bgm.mp3");
            startBackgroundMusic("lrc.mp3");
            printf(GREEN"\n=== Library Lockdown Event! ===\n"RESET);
            sleep(1);
            printf("The library is packed with stressed students preparing for exams...\n");
            sleep(1);

            if (!story.library_secured)
            {
                printf("You'll need to fight for a study spot!\n");
                sleep(2);
                if (combat("Stressed Senior", 0))
                {
                    story.library_secured = 1;
                    printf("You've secured a prime study spot!\n");
                    player.intelligence += 5;
                    player.exam_prep += 20;
                }
            }
            sleep(3);
            stopBackgroundMusic("lrc.mp3");
            startBackgroundMusic("bgm.mp3");
        }

        else if (strcmp(locations[choice].name, "Dean's Office") == 0 &&
                 story.act == 4 && !story.deanDefeated)
        {   stopBackgroundMusic("bgm.mp3");
            startBackgroundMusic("boss.mp3");
            printf(GREEN"\n=== Final Boss Battle! ===\n"RESET);
            sleep(1);
            printf("The Dean stands before you, ready to defend his corrupt empire!\n");
            sleep(2);
            if (combat("The Dean", 2))
            {
                story.deanDefeated = 1;
                printf("\nCongratulations! You've defeated the Dean and exposed the corruption!\n");
                printf("You are now a campus legend!\n");
                player.experience += 1000;
                player.money += 5000;
            }
            sleep(4);
            stopBackgroundMusic("boss.mp3");
            startBackgroundMusic("bgm.mp3");
        }

        player.stress += locations[choice].stress_modifier;
        if (player.stress < 0)
            player.stress = 0;
        if (player.stress > 100)
            player.stress = 100;

        printf("\nYou've moved to %s\n", locations[choice].name);
        printf("Stress %s by %d\n",
               locations[choice].stress_modifier > 0 ? "increased" : "decreased",
               abs(locations[choice].stress_modifier));

        if (locations[choice].study_bonus != 0)
        {
            printf("Study effectiveness %s by %d%%\n",
                   locations[choice].study_bonus > 0 ? "increased" : "decreased",
                   abs(locations[choice].study_bonus));
        }
    }
    else if (choice != -1)
    {
        printf("Invalid location choice!\n");
    }
}
void socialize()
{
    if (player.energy < 10)
    {
        printf("You're too tired to socialize!\n");
        return;
    }

    printf(GREEN"\n=== Social Activities ===\n"RESET);
    printf("1. Chat with Friends");
    printf("\n2. Join a Club Event");
    printf("\n3. Organize a Study Group");
    printf("\n4. Attend a Party");
    printf("\n0. Back\n");
    printf("\nChoice: ");

    char input[10];
    fgets(input, sizeof(input), stdin);
    int choice = atoi(input);

    switch (choice)
    {
    case 1:
    {
        int chance = rand() % 100;
        player.energy -= 10;

        if (chance < 70)
        {
            printf("\nYou had a great conversation about the upcoming events!\n");
            player.charisma += 2;
            player.stress -= 10;

            if (story.act == 2 && !story.dance_battle_won && player.charisma >= 20)
            {   stopBackgroundMusic("bgm.mp3");
                startBackgroundMusic("dance.mp3");
                printf("\nMisky notices your growing popularity...\n");
                sleep(1);
                printf("She challenges you to a dance battle!\n");
                sleep(2);
                if (combat("Misky", 1))
                {
                    story.dance_battle_won = 1;
                    player.dance_skill += 10;
                    printf("You're now the dance champion!\n");
                }
                sleep(3);
                stopBackgroundMusic("dance.mp3");
                startBackgroundMusic("bgm.mp3");
            }
            if (!story.corruption_exposed && story.act == 4)
            {   
                printf("\nYou discover some interesting gossip about the Dean's activities...\n");
                story.corruption_exposed = 1;
                printf("You've uncovered evidence of corruption!\n");
                player.experience += 300;
            }

        }
        else
        {
            printf("\nYou make a new friend!\n");
            player.charisma += 3;
            player.stress -= 15;
        }
        break;
    }

    case 2:
        if (clubCount > 0)
        {
            printf("\nAvailable Club Events:\n");
            for (int i = 0; i < clubCount; i++)
            {
                if (clubs[i].joined)
                {
                    printf("%d. %s Meeting\n", i + 1, clubs[i].name);
                }
            }
        }
        else
        {
            printf("You haven't joined any clubs yet!\n");
        }
        break;

    case 3:
    {
        printf("\nOrganizing a study group...\n");
        player.energy -= 20;
        player.exam_prep += 15;
        player.intelligence += 2;
        player.charisma += 1;
        printf("The study session was productive!\n");
        printf("Exam Prep +15 | Intelligence +2 | Charisma +1\n");
        break;
    }

    case 4:
    {
        if (player.money < 100)
        {
            printf("You can't afford to attend the party! (Cost: ₹100)\n");
            return;
        }

        printf("\nAttending a college party...\n");
        player.money -= 100;
        player.energy -= 30;
        player.stress -= 25;
        player.charisma += 5;

        int event = rand() % 3;
        switch (event)
        {
        case 0:
            printf("You became the life of the party!\n");
            player.charisma += 3;
            break;
        case 1:
            printf("You networked with seniors and gained valuable advice!\n");
            player.intelligence += 2;
            break;
        case 2:
            printf("You got caught by Professor Pataka!\n");
            player.stress += 20;
            break;
        }
        break;
    }
    }
}

void displayStoryProgress()
{
    printf(RED BOLD "\n=== Current Story Progress ===\n"RESET);
    printf(CYAN "Act %d\n" RESET, story.act);

    switch (story.act)
    {
    case 1:
        printf(RED"Chapter: Freshman Beginnings\n" RESET);
        printf(RED"Goal: Establish yourself in college life\n"RESET);
        break;
    case 2:
        printf(RED"Chapter: Social Butterfly\n"RESET);
        printf(RED"Goal: Rise in campus social hierarchy\n"RESET);
        break;
    case 3:
        printf(RED"Chapter: Academic Challenges\n"RESET);
        printf(RED"Goal: Secure your position in the library\n"RESET);
        break;
    case 4:
        printf(RED"Chapter: Campus Corruption\n"RESET);
        printf(RED"Goal: Expose the truth and confront the Dean\n"RESET);
        break;
    }
}

void displayActOptions(int act)
{
    printf(BLUE "\nAvailable Actions:\n" RESET);
    printf(MAGENTA"1. Check Status\n"RESET);
    printf(MAGENTA"2. Eat\n"RESET);
    printf(MAGENTA"3. Sleep\n"RESET);

    switch (act)
    {
    case 1:
        printf(MAGENTA"4. Study\n"RESET);
        printf(MAGENTA"5. Join Club\n"RESET);
        printf(MAGENTA"6. Talk to Stranger\n"RESET);
        printf(MAGENTA"7. Change Location\n"RESET);
        break;

    case 2:
        printf(MAGENTA"4. Socialize\n"RESET);
        printf(MAGENTA"5. Exercise\n"RESET);
        printf(MAGENTA"6. Change Location\n"RESET);
        printf(MAGENTA"7. Attend Workshop\n"RESET);
        break;

    case 3:
        printf(MAGENTA"4. Study\n"RESET);
        printf(MAGENTA"5. Track Exam\n"RESET);
        printf(MAGENTA"6. Change Location\n"RESET);
        printf(MAGENTA"7. View Quests\n"RESET);
        printf(MAGENTA"8. Manage Finances\n"RESET);
        break;

    case 4:
        printf(MAGENTA"4. Socialize (Gather Intel)\n"RESET);
        printf(MAGENTA"5. Change Location\n"RESET);
        printf(MAGENTA"6. Manage Finances\n"RESET);
        printf(MAGENTA"7. View Evidence\n"RESET);

        if (story.corruption_exposed)
        {
            printf(MAGENTA"8. Confront Dean\n"RESET);
        }

        if (story.deanDefeated)
        {
            printf(CYAN BOLD"\n=== Congratulations! You've won the game! ===\n"RESET);
            printf(CYAN"You've become a campus legend and saved the university!\n"RESET);
        }
        break;
    }
    printf(MAGENTA"0. Quit Game\n"RESET);
}

void checkActProgression()
{
    int previousAct = story.act;

    switch (story.act)
    {
    case 1:
        if (player.charisma >= 15 && player.intelligence >= 15)
        {
            story.act = 2;
            printf(CYAN BOLD"\n=== Congratulations! You've advanced to Act 2! ===\n"RESET);
            printf(CYAN"You've gained enough confidence to enter the social scene!\n"RESET);
            printf("\nPress Enter to continue...");
            getchar();
        }
        break;

    case 2:
        if (story.dance_battle_won)
        {
            story.act = 3;
            printf(CYAN BOLD"\n=== Congratulations! You've advanced to Act 3! ===\n"RESET);
            printf(CYAN"Your social achievements have earned you respect. Time to focus on academics!\n"RESET);
            printf("\nPress Enter to continue...");
            getchar();
        }
        break;

    case 3:
        if (story.library_secured && player.exam_prep >= 50)
        {
            story.act = 4;
            printf(CYAN BOLD"\n=== Congratulations! You've advanced to Act 4! ===\n" RESET);
            printf(CYAN BOLD"While studying, you've discovered hints of corruption...\n"RESET);
            printf("\nPress Enter to continue...");
            getchar();
        }
        break;
    }

    if (previousAct != story.act)
    {
        clearScreen();
    }
}

void handleActChoice(int act, int choice)
{
    clearScreen(); 

    if (choice == 1)
        printStatus();
    else if (choice == 2)
        eat();
    else if (choice == 3)
        mysleep();
    else if (choice == 0)
    {
        stopBackgroundMusic("bgm.mp3");
        printf(GREEN"Thanks for playing! Goodbye!\n"RESET);
        exit(0);
    }

    switch (act)
    {
    case 1:
        switch (choice)
        {
        case 4:
            study();
            break;
        case 5:
            joinClub();
            break;
        case 6:
            strangerTalk();
            break;
        case 7:
            changeLocation();
            break;
        }
        break;

    case 2:
        switch (choice)
        {
        case 4:
            socialize();
            break;
        case 5:
            exercise();
            break;
        case 6:
            changeLocation();
            break;
        case 7:
            attendWorkshop();
            break;
        }
        break;

    case 3:
        switch (choice)
        {
        case 4:
            study();
            break;
        case 5:
            trackExam();
            break;
        case 6:
            changeLocation();
            break;
        case 7:
            printQuests();
            break;
        case 8:
            manageFinances();
            break;
        }
        break;

    case 4:
        switch (choice)
        {
        case 4:
            socialize();
            break;
        case 5:
            changeLocation();
            break;
        case 6:
            manageFinances();
            break;
        case 7:
            if (story.corruption_exposed)
            {
                printf("\nCollected Evidence:\n");
                printf("- Suspicious financial records\n");
                printf("- Witness testimonies\n");
                printf("- Documented irregularities\n");
            }
            else
            {
                printf("\nNo evidence collected yet.\n");
            }
            break;
        case 8:
            if (story.corruption_exposed && !story.deanDefeated)
            {
                printf("\nConfronting the Dean...\n");
                changeLocation();
            }
            break;
        }
        break;
    }

    printf("\nPress Enter to continue...");
    getchar(); 
    clearScreen();
}

void gameLoop()
{
    char input[20];
    story.act = 1; 

    while (1)
    {
        clearScreen();
        displayStoryProgress();
        displayActOptions(story.act);
        printf("\nEnter your choice: ");
        fgets(input, sizeof(input), stdin);
        int choice = atoi(input);

        handleActChoice(story.act, choice);
        checkActProgression();

        if (story.deanDefeated && story.act == 4)
        {
            clearScreen();
            printf(CYAN BOLD"\n===== Congratulations! You've completed College Life RPG! =====\n"RESET);
            printf(CYAN "You've become a campus legend and saved the university!\n"RESET);
            printf(CYAN "\nFinal Stats:\n"RESET);
            printStatus();
            printf("\nPress Enter to exit...");
            getchar();
            break;
        }
    }
}

int main()
{
    login_signup();
    srand(time(NULL));
    initCharacter();
    initLocations();
    startBackgroundMusic("bgm.mp3");
    printascii("logo.txt");
    addQuest(GREEN"Complete Assignment", "Submit your assignment by end of week."RESET, 20);
    initClubs();
    addQuest(GREEN"Meet New Friends", "Increase charisma by socializing."RESET, 10);
    sleep(5);
    gameLoop();
    return 0;
}
