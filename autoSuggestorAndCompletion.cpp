/*
    Aurthor: Surya Bahadur Basnet

    # AutoSuggestor and Completion for cross platform.
    # Auto Completion on TAB press.
    Tested in linux-> Manjaro and windows->11
    // Add any word you want to dictionary.txt file
*/

#include <iostream>
#include <fstream>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#elif __linux__ || __APPLE__
    #include <termios.h>
    #include <unistd.h>
#endif 

#define ALPHABET_SIZE 60
#define MAX_SUGGESTION_SIZE 4
#define DICTIONARY_FILE_NAME "dictionary.txt"

using namespace std; 
class TrieNode {
public:
    TrieNode* children[ALPHABET_SIZE]; 
    bool isEndOfWord;
    TrieNode() {
        for (int i = 0; i < ALPHABET_SIZE; ++i) {
            children[i] = nullptr;
        }
        isEndOfWord = false;
    }
};

// helpers
int getArrayLength(const string*);
void clearScreen();

// Functioning Stuff
void insert(TrieNode*, string);
void possibleTextHelper(TrieNode*, string, string*, int&);
string* search(TrieNode* , const string&);

// Main Stuff
string* getSuggestions(TrieNode*, string, string*);
void InsertDictionary(TrieNode&, string);

#ifdef _WIN32
    void windowsOperation(TrieNode&);
# elif __linux__ || __APPLE__
    void nonWindowsOperation(TrieNode&);
#endif

// Main Function
int main(){
    TrieNode autoCompletionNode;
    InsertDictionary(autoCompletionNode, DICTIONARY_FILE_NAME);

    // insert (&autoCompletionNode, "apple");
    // insert (&autoCompletionNode, "an");

    cout<< ">> ";
    #ifdef __WIN32
        windowsOperation(autoCompletionNode);
    #elif __linux__ || __APPLE__
        nonWindowsOperation(autoCompletionNode);
    #endif
    return 0;
}

// For Main Operation to take input
#ifdef _WIN32
void windowsOperation(TrieNode& autoCompletionNode){
    string userInput = "";
    // One character at a time setup
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode, numRead;
    INPUT_RECORD input;


    string* result = new string[MAX_SUGGESTION_SIZE];
    while (69) {
        string suggestionFormat = "";

        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);
        ReadConsoleInput(hStdin, &input, 1, &numRead);
        SetConsoleMode(hStdin, mode);
        bool shiftKeyPressed = false;
        
        if (input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown) {
            if (input.Event.KeyEvent.wVirtualKeyCode == VK_TAB){
                userInput = result[0];
            }
            else if (input.Event.KeyEvent.wVirtualKeyCode == VK_SHIFT){
                shiftKeyPressed = true;
            }
            else if (input.Event.KeyEvent.wVirtualKeyCode == VK_BACK){
                userInput = userInput.substr(0, userInput.length()-1);
                if (userInput != ""){
                    result = getSuggestions(&autoCompletionNode, userInput, &suggestionFormat);
                }
            }
            else {
                char inputChar = input.Event.KeyEvent.uChar.AsciiChar;
                inputChar = shiftKeyPressed? toupper(inputChar) : inputChar;
                shiftKeyPressed = false;
                if (inputChar == '\r') {
                    break;
                }
                userInput+=inputChar;

                //  autocomplete
                delete[] result;
                result = getSuggestions(&autoCompletionNode, userInput, &suggestionFormat);
            }
            clearScreen();
            cout << ">> " << userInput << "\n" << suggestionFormat;
        }
    }
    delete[] result;
}

#elif __linux__ || __APPLE__
void nonWindowsOperation(TrieNode& autoCompletionNode){
    string userInput = "";
    // One character at a time setup
    struct termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    string* result = new string[MAX_SUGGESTION_SIZE];
    while (69) {
        string suggestionFormat = "";

        char inputChar = getchar();
        if (inputChar == '\n') {
            tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
            break;
        }

        else if (inputChar == '\t') {
            userInput = result[0];
        }

        else if (inputChar == 8 || inputChar == 127) {
            userInput = userInput.substr(0, userInput.length() - 1);
            if (userInput != "") {
                result = getSuggestions(&autoCompletionNode, userInput, &suggestionFormat);
            }
        }
        else {
            userInput += inputChar;
            delete[] result;
            result = getSuggestions(&autoCompletionNode, userInput, &suggestionFormat);
        }
        clearScreen();
        cout <<">> " << userInput << "\n" << suggestionFormat;
    }
}
#endif


// for available suggestion, max 3
string* getSuggestions(TrieNode* root, string key, string* suggestionFormat){
    string* result = search(root, key);
    const int arrSize = getArrayLength(result);
    for (int i=0; i< arrSize; i++){
        *suggestionFormat+= result[i] + ", ";
    }
    return result;
}

// Add list of words to TrieNode
void InsertDictionary(TrieNode& autoCompletionNode, string fileName){
    ifstream dictionary(fileName);
    if (!dictionary.is_open()) {
        cerr << "Failed to open the file! \n";
        exit(0);
    }
    string word;
    while (dictionary >> word) {
        insert(&autoCompletionNode, word);
    }
    if (!dictionary.eof() && !dictionary.good()) {
        cerr << "Error: could not read file \n";
        dictionary.close();
        exit(0);
    }
    dictionary.close();
}

// Insert word to TrieNode
void insert(TrieNode* root, string key){
    TrieNode* currentNode = root;
    for (char curChar: key){
        const int index = curChar - 'A';
        if (currentNode->children[index] == nullptr){
            currentNode->children[index] = new TrieNode();
        }
        currentNode = currentNode->children[index];
    }
    currentNode->isEndOfWord = true;
}

// get all PossibleText from TrieNode, Basically get upto 3(max) or any number under
void possibleTextHelper(TrieNode* node, string key, string* possibleTexts, int& idx){
    if (idx>=3 || node == nullptr ) return;
    if (node->isEndOfWord){
        possibleTexts[idx++] = key;
    }
    for (int i=0; i<ALPHABET_SIZE; i++){
        if (node->children[i] != nullptr){
        char c = 'A'+i;
        possibleTextHelper(node->children[i], key+c, possibleTexts, idx);
        }
    }
}

// search for possible words, calls -> possibleTextHelper for so.
string* search(TrieNode* root, const string& key) {
    TrieNode* currentNode = root;
    for (char c : key) {
        int index = c - 'A';
        if (currentNode->children[index] == nullptr) {
            return new string[3]();
        }
        currentNode = currentNode->children[index];
    }
    string* possibleTexts = new string[MAX_SUGGESTION_SIZE];
    int idx = 0;
    possibleTextHelper(currentNode, key, possibleTexts, idx);
    return possibleTexts;
}

// Helper Functions.
int getArrayLength(const string* arrPtr) {
    int count = 0;
    while (arrPtr[count] != ""){
        count++;
    }
    return count;
}
void clearScreen(){
    cout << "\033[2J\033[1;1H"; // Ascii key to clear console. [Platform Independent]
}
