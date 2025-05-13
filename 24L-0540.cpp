#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
using namespace std;


class User;
class Investor;
class BusinessOwner;
class MatchMaker;
class Platform;

enum class Category {
    TECHNOLOGY,
    HEALTHCARE,
    FINANCE,
    RETAIL,
    MANUFACTURING,
    EDUCATION,
    OTHER
};

string categoryToString(Category category) {
    switch (category) {
    case Category::TECHNOLOGY: return "Technology";
    case Category::HEALTHCARE: return "Healthcare";
    case Category::FINANCE: return "Finance";
    case Category::RETAIL: return "Retail";
    case Category::MANUFACTURING: return "Manufacturing";
    case Category::EDUCATION: return "Education";
    case Category::OTHER: return "Other";
    default: return "Unknown";
    }
}

Category stringToCategory(const string& categoryStr) {
    if (categoryStr == "Technology") return Category::TECHNOLOGY;
    if (categoryStr == "Healthcare") return Category::HEALTHCARE;
    if (categoryStr == "Finance") return Category::FINANCE;
    if (categoryStr == "Retail") return Category::RETAIL;
    if (categoryStr == "Manufacturing") return Category::MANUFACTURING;
    if (categoryStr == "Education") return Category::EDUCATION;
    return Category::OTHER;
}

class User {
protected:
    string username;
    string password;
    string name;

    string email;
    bool isInvestor;

public:
    User(string username, string password, string name, string email, bool isInvestor)

        : username(username), password(password), name(name), email(email), isInvestor(isInvestor) {
    }

    virtual ~User() = default;

    bool authenticate(string inputPassword) const {
        return password == inputPassword;
    }

    string getUsername() const { return username; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    bool getIsInvestor() const { return isInvestor; }
    string getPassword() const { return password; }


    virtual void display() const = 0;
    virtual string serialize() const = 0;
};


class Investor : public User {
private:
    double investmentCapacity;
    Category interestedCategory;
    string investmentGoals;

public:
    Investor(string username, string password,
        string name, string email,
        double investmentCapacity, Category category,

        string goals)
        : User(username, password, name, email, true),
        investmentCapacity(investmentCapacity),
        interestedCategory(category),
        investmentGoals(goals) {
    }

    double getInvestmentCapacity() const { return investmentCapacity; }
    Category getInterestedCategory() const { return interestedCategory; }
    string getInvestmentGoals() const { return investmentGoals; }

    void display() const override {
        cout << "Investor: " << name << "\n";

        cout << "Investment Capacity: $" << investmentCapacity << "\n";
        cout << "Goals: " << investmentGoals << "\n";
    }

    string serialize() const override {
        return "I|" + username + "|" + password + "|" + name + "|" + email + "|" +
            to_string(investmentCapacity) + "|" +
            categoryToString(interestedCategory) + "|" + investmentGoals;
    }

    static Investor* deserialize(const string& data) {
        string parts[8];
        size_t start = data.find('|', 0) + 1;
        size_t end = 0;
        int i = 0;

        while (i < 7 && start != string::npos) {
            end = data.find('|', start);
            if (end == string::npos) {
                parts[i++] = data.substr(start);
                break;
            }
            parts[i++] = data.substr(start, end - start);
            start = end + 1;
        }

        if (i < 7 || parts[6].empty()) {
            parts[6] = data.substr(start);
        }

        return new Investor(
            parts[0],
            parts[1],
            parts[2],
            parts[3],
            stod(parts[4]),
            stringToCategory(parts[5]),
            parts[6]
        );
    }
};


class BusinessOwner : public User {
private:
    string businessName;

    string businessDescription;
    double marketSales;
    double investmentNeeded;
    Category category;

public:
    BusinessOwner(string username, string password,
        string name, string email,
        string businessName, string description,

        double marketSales, double investmentNeeded,
        Category category)
        : User(username, password, name, email, false),
        businessName(businessName), businessDescription(description),
        marketSales(marketSales), investmentNeeded(investmentNeeded),
        category(category) {
    }

    string getBusinessName() const { return businessName; }
    string getBusinessDescription() const { return businessDescription; }
    double getMarketSales() const { return marketSales; }
    double getInvestmentNeeded() const { return investmentNeeded; }
    Category getCategory() const { return category; }

    void display() const override {
        cout << "Business: " << businessName << "\n";
        cout << "Owner: " << name << "\n";

        cout << "Sales: $" << marketSales << "\n";
        cout << "Investment Needed: $" << investmentNeeded << "\n";
    }

    string serialize() const override {
        return "B|" + username + "|" + password + "|" + name + "|" + email + "|" +
            businessName + "|" + businessDescription + "|" +
            to_string(marketSales) + "|" + to_string(investmentNeeded) + "|" +
            categoryToString(category);
    }

    static BusinessOwner* deserialize(const string& data) {
        string parts[10];
        size_t start = data.find('|', 0) + 1;
        size_t end = 0;
        int i = 0;

        while (i < 9 && start != string::npos) {
            end = data.find('|', start);
            if (end == string::npos) {
                parts[i++] = data.substr(start);
                break;
            }
            parts[i++] = data.substr(start, end - start);
            start = end + 1;
        }

        if (i < 9 || parts[8].empty()) {
            parts[8] = data.substr(start);
        }

        return new BusinessOwner(
            parts[0],
            parts[1],
            parts[2],
            parts[3],
            parts[4],
            parts[5],
            stod(parts[6]),
            stod(parts[7]),
            stringToCategory(parts[8])
        );
    }
};


class MatchMaker {
public:
    static bool isMatch(const Investor& investor, const BusinessOwner& business) {
        return (investor.getInterestedCategory() == business.getCategory() &&
            investor.getInvestmentCapacity() >= business.getInvestmentNeeded());
    }
};

class FileManager {
public:
    static bool saveUsers(User* users[], int userCount, const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Could not open file for writing: " << filename << endl;
            return false;

        }

        for (int i = 0; i < userCount; i++) {
            file << users[i]->serialize() << endl;
        }

        file.close();
        return true;
    }

    static int loadUsers(User* users[], const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "No existing user data file found." << endl;
            return 0;
        }

        int userCount = 0;
        string line;

        while (getline(file, line) && userCount < 100) {
            if (line.empty()) continue;

            if (line[0] == 'I') {
                users[userCount++] = Investor::deserialize(line);
            }
            else if (line[0] == 'B') {
                users[userCount++] = BusinessOwner::deserialize(line);
            }
        }

        file.close();
        return userCount;
    }
};


class Platform {
private:
    sf::RenderWindow window;
    sf::Font font;
    User* users[100];
    int userCount;
    User* currentUser;

    enum class State {
        LOGIN,
        LOGIN_AUTHENTICATE,

        REGISTER,
        INVESTOR_SIGNUP,
        BUSINESS_SIGNUP,
        INVESTOR_DASHBOARD,
        BUSINESS_DASHBOARD,
        MATCHING_RESULTS
    };

    State currentState;
    sf::Text headerText;
    sf::Text instructionText;
    sf::Text notificationText;
    sf::RectangleShape inputBox;
    string currentInput;

    // Form fields
    string formUsername;
    string formPassword;
    string formName;
    string formEmail;
    double formInvestmentCapacity;
    Category formCategory;
    string formGoals;
    string formBusinessName;

    string formBusinessDesc;
    double formMarketSales;
    double formInvestmentNeeded;

    int currentFormField;
    float notificationTimer;

    const string DATA_FILE = "data.txt";


public:
    Platform() : window(sf::VideoMode(800, 600), "Investment Matching Platform"),
        userCount(0), currentUser(nullptr), currentState(State::LOGIN),
        currentFormField(0), notificationTimer(0.0f) {

        if (!font.loadFromFile("Jaro-Regular-VariableFont_opsz.ttf")) {

            cout << "Error loading font\n";
        }

        headerText.setFont(font);
        headerText.setCharacterSize(24);
        headerText.setPosition(20, 20);

        instructionText.setFont(font);
        instructionText.setCharacterSize(18);
        instructionText.setPosition(20, 80);

        notificationText.setFont(font);
        notificationText.setCharacterSize(16);
        notificationText.setPosition(20, 530);
        notificationText.setFillColor(sf::Color::Yellow);

        inputBox.setSize(sf::Vector2f(400, 40));
        inputBox.setPosition(20, 320);

        inputBox.setFillColor(sf::Color::White);
        inputBox.setOutlineColor(sf::Color::Black);
        inputBox.setOutlineThickness(2);

        loadUsersFromFile();
    }

    ~Platform() {
        saveUsersToFile();

        for (int i = 0; i < userCount; i++) {
            delete users[i];
        }
    }

    void run() {
        sf::Clock clock;

        while (window.isOpen()) {
            sf::Time deltaTime = clock.restart();
            processEvents();
            update(deltaTime);
            render();
        }
    }

private:
    void loadUsersFromFile() {
        userCount = FileManager::loadUsers(users, DATA_FILE);
        showNotification("Loaded " + to_string(userCount) + " users from file.");
    }

    void saveUsersToFile() {
        if (FileManager::saveUsers(users, userCount, DATA_FILE)) {
            cout << "Users saved successfully." << endl;
        }
        else {
            cout << "Failed to save users." << endl;
        }
    }

    void showNotification(const string& message) {
        notificationText.setString(message);
        notificationTimer = 3.0f;

    }

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    handleTextInput(static_cast<char>(event.text.unicode));
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Return) {
                    handleEnterKey();
                }
                else if (event.key.code == sf::Keyboard::BackSpace) {
                    if (!currentInput.empty()) {
                        currentInput.pop_back();
                    }
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    handleEscapeKey();
                }
            }
        }
    }

    void handleEscapeKey() {
        if (currentState == State::MATCHING_RESULTS) {
            if (currentUser->getIsInvestor()) {
                currentState = State::INVESTOR_DASHBOARD;
            }
            else {
                currentState = State::BUSINESS_DASHBOARD;
            }
        }
        else if (currentState == State::INVESTOR_DASHBOARD ||
            currentState == State::BUSINESS_DASHBOARD) {
            currentUser = nullptr;
            currentState = State::LOGIN;
            showNotification("Logged out successfully.");

        }
    }

    void handleTextInput(char input) {
        if (input >= 32 && input <= 126) {
            currentInput += input;
        }
    }

    void handleEnterKey() {
        switch (currentState) {
        case State::LOGIN:
            handleLoginEnter();
            break;

        case State::LOGIN_AUTHENTICATE:
            handleLoginAuthEnter();
            break;

        case State::REGISTER:
            handleRegisterEnter();

            break;

        case State::INVESTOR_SIGNUP:
            handleInvestorSignupForm();
            break;

        case State::BUSINESS_SIGNUP:
            handleBusinessSignupForm();
            break;

        case State::INVESTOR_DASHBOARD:
        case State::BUSINESS_DASHBOARD:
            handleDashboardEnter();
            break;

        default:
            break;
        }
    }

    void handleLoginEnter() {

        for (int i = 0; i < userCount; i++) {
            if (users[i]->getUsername() == currentInput) {
                formUsername = currentInput;
                currentState = State::LOGIN_AUTHENTICATE;
                currentInput = "";
                return;
            }
        }

        if (currentInput == "new") {
            currentState = State::REGISTER;
        }
        else {
            showNotification("Username not found. Type 'new' to register.");
        }
        currentInput = "";
    }

    void handleLoginAuthEnter() {

        for (int i = 0; i < userCount; i++) {
            if (users[i]->getUsername() == formUsername && users[i]->authenticate(currentInput)) {
                currentUser = users[i];
                if (currentUser->getIsInvestor()) {
                    currentState = State::INVESTOR_DASHBOARD;
                }
                else {
                    currentState = State::BUSINESS_DASHBOARD;
                }
                showNotification("Login successful! Welcome, " + currentUser->getName());
                break;
            }
        }

        if (currentUser == nullptr) {
            showNotification("Incorrect password.");
            currentState = State::LOGIN;
        }
        currentInput = "";
    }

    void handleRegisterEnter() {

        if (currentInput == "1") {
            currentState = State::INVESTOR_SIGNUP;
        }
        else if (currentInput == "2") {
            currentState = State::BUSINESS_SIGNUP;
        }
        currentInput = "";
        currentFormField = 0;
    }

    void handleDashboardEnter() {

        if (currentInput == "1") {
            currentState = State::MATCHING_RESULTS;
            showNotification("Press ESC to return to dashboard");

        }
        else if (currentInput == "2") {
            saveUsersToFile();
            showNotification("Data saved successfully!");
        }
        currentInput = "";

    }

    void handleInvestorSignupForm() {
        switch (currentFormField) {
        case 0: // Username
            if (!validateUsername()) return;

            formUsername = currentInput;
            break;
        case 1: // Password
            formPassword = currentInput;
            break;
        case 2: // Name
            formName = currentInput;
            break;
        case 3: // Email
            formEmail = currentInput;
            break;
        case 4: // Investment capacity
            if (!validateNumber(formInvestmentCapacity)) return;

            break;
        case 5: // Category(type)
            if (!validateCategory()) return;

            break;
        case 6: // targets and goals
            formGoals = currentInput;
            users[userCount] = new Investor(
                formUsername, formPassword, formName, formEmail,
                formInvestmentCapacity, formCategory, formGoals
            );
            userCount++;

            currentUser = users[userCount - 1];
            currentState = State::INVESTOR_DASHBOARD;
            showNotification("Registration successful!");
            break;
        }

        currentInput = "";
        currentFormField++;
    }

    bool validateUsername() {

        for (int i = 0; i < userCount; i++) {
            if (users[i]->getUsername() == currentInput) {
                showNotification("Username already exists. Please choose another.");
                return false;
            }
        }
        return true;
    }

    bool validateNumber(double& value) {
        try {
            value = stod(currentInput);
            return true;
        }
        catch (const exception& e) {
            showNotification("Please enter a valid number.");
            return false;
        }
    }

    bool validateCategory() {
        try {
            int catNum = stoi(currentInput);
            if (catNum < 1 || catNum > 7) {
                showNotification("Please enter a number between 1 and 7.");
                return false;
            }
            formCategory = static_cast<Category>(catNum - 1);
            return true;
        }
        catch (const exception& e) {
            showNotification("Please enter a valid number.");
            return false;

        }
    }

    void handleBusinessSignupForm() {
        switch (currentFormField) {
        case 0: // Username
            if (!validateUsername()) return;

            formUsername = currentInput;
            break;
        case 1: // Password
            formPassword = currentInput;
            break;
        case 2: // Name
            formName = currentInput;
            break;
        case 3: // Email
            formEmail = currentInput;
            break;
        case 4: // Business name
            formBusinessName = currentInput;
            break;
        case 5: // Business description
            formBusinessDesc = currentInput;
            break;
        case 6: // Market sales
            if (!validateNumber(formMarketSales)) return;
            break;
        case 7: // Investment needed
            if (!validateNumber(formInvestmentNeeded)) return;

            break;
        case 8: // Category(Type)
            if (!validateCategory()) return;


            users[userCount] = new BusinessOwner(
                formUsername, formPassword, formName, formEmail,
                formBusinessName, formBusinessDesc, formMarketSales,
                formInvestmentNeeded, formCategory
            );
            userCount++;

            currentUser = users[userCount - 1];
            currentState = State::BUSINESS_DASHBOARD;
            showNotification("Registration successful!");
            break;
        }

        currentInput = "";
        currentFormField++;
    }

    void update(sf::Time deltaTime) {
        // Update notification timer
        if (notificationTimer > 0) {
            notificationTimer -= deltaTime.asSeconds();
            if (notificationTimer <= 0) {
                notificationText.setString("");

            }
        }

        // Update UI text based on current state
        updateUIText();
    }

    void updateUIText() {

        switch (currentState) {
        case State::LOGIN:
            headerText.setString("Investment Platform Login");
            instructionText.setString("Enter username (or 'new' to register):");
            break;

        case State::LOGIN_AUTHENTICATE:

            headerText.setString("Investment Platform Login");
            instructionText.setString("Enter password for " + formUsername + ":");

            break;

        case State::REGISTER:
            headerText.setString("Registration");
            instructionText.setString("Select account type:\n1. Investor\n2. Business Owner");
            break;

        case State::INVESTOR_SIGNUP:
            headerText.setString("Investor Registration");
            updateInvestorSignupText();

            break;

        case State::BUSINESS_SIGNUP:
            headerText.setString("Business Registration");
            updateBusinessSignupText();

            break;

        case State::INVESTOR_DASHBOARD:
            headerText.setString("Investor Dashboard");
            instructionText.setString("Welcome, " + currentUser->getName() +
                "\n\nPress 1 to view matches" +
                "\nPress 2 to save data" +
                "\nPress ESC to logout");

            break;

        case State::BUSINESS_DASHBOARD:
            headerText.setString("Business Dashboard");
            instructionText.setString("Welcome, " + currentUser->getName() +
                "\n\nPress 1 to view potential investors" +
                "\nPress 2 to save data" +
                "\nPress ESC to logout");

            break;

        case State::MATCHING_RESULTS:
            headerText.setString("Matching Results");
            findMatches();
            break;
        }
    }

    void updateInvestorSignupText() {

        switch (currentFormField) {
        case 0: instructionText.setString("Enter username:"); break;
        case 1: instructionText.setString("Enter password:"); break;
        case 2: instructionText.setString("Enter your name:"); break;
        case 3: instructionText.setString("Enter your email:"); break;
        case 4: instructionText.setString("Enter investment capacity ($):"); break;
        case 5: instructionText.setString("Select category (1-7):\n1. Tech 2. Healthcare 3. Finance\n4. Retail 5. Manufacturing 6. Education 7. Other"); break;
        case 6: instructionText.setString("Enter investment goals:"); break;
        }
    }

    void updateBusinessSignupText() {

        switch (currentFormField) {
        case 0: instructionText.setString("Enter username:"); break;
        case 1: instructionText.setString("Enter password:"); break;
        case 2: instructionText.setString("Enter your name:"); break;
        case 3: instructionText.setString("Enter your email:"); break;
        case 4: instructionText.setString("Enter business name:"); break;
        case 5: instructionText.setString("Enter business description:"); break;
        case 6: instructionText.setString("Enter market sales ($):"); break;
        case 7: instructionText.setString("Enter investment needed ($):"); break;
        case 8: instructionText.setString("Select category (1-7):\n1. Tech 2. Healthcare 3. Finance\n4. Retail 5. Manufacturing 6. Education 7. Other"); break;
        }
    }

    void findMatches() {

        string matchText = "Potential matches:\n\n";

        if (currentUser->getIsInvestor()) {
            Investor* investor = static_cast<Investor*>(currentUser);

            for (int i = 0; i < userCount; i++) {
                if (!users[i]->getIsInvestor()) {
                    BusinessOwner* business = static_cast<BusinessOwner*>(users[i]);

                    if (MatchMaker::isMatch(*investor, *business)) {
                        matchText += "Business: " + business->getBusinessName() + "\n";
                        matchText += "Description: " + business->getBusinessDescription() + "\n";
                        matchText += "Category: " + categoryToString(business->getCategory()) + "\n";
                        matchText += "Investment Needed: $" + to_string(int(business->getInvestmentNeeded())) + "\n\n";
                    }
                }
            }
        }
        else {

            BusinessOwner* business = static_cast<BusinessOwner*>(currentUser);

            for (int i = 0; i < userCount; i++) {
                if (users[i]->getIsInvestor()) {
                    Investor* investor = static_cast<Investor*>(users[i]);

                    if (MatchMaker::isMatch(*investor, *business)) {
                        matchText += "Investor: " + investor->getName() + " (" + investor->getEmail() + ")\n";
                        matchText += "Investment Capacity: $" + to_string(int(investor->getInvestmentCapacity())) + "\n";
                        matchText += "Category Interest: " + categoryToString(investor->getInterestedCategory()) + "\n";

                        matchText += "Goals: " + investor->getInvestmentGoals() + "\n\n";
                    }
                }
            }
        }

        if (matchText == "Potential matches:\n\n") {
            matchText += "No matches found.";
        }

        matchText += "\n\nPress ESC to return to dashboard";
        instructionText.setString(matchText);
    }

    void render() {
        window.clear(sf::Color(42, 54, 59));

        window.draw(headerText);
        window.draw(instructionText);

        if (!notificationText.getString().isEmpty()) {
            window.draw(notificationText);

        }

        if (currentState != State::MATCHING_RESULTS &&
            currentState != State::INVESTOR_DASHBOARD &&
            currentState != State::BUSINESS_DASHBOARD) {
            window.draw(inputBox);

            sf::Text inputText;
            inputText.setFont(font);
            inputText.setString(currentInput);
            inputText.setCharacterSize(18);
            inputText.setFillColor(sf::Color::Black);
            inputText.setPosition(30, 330);

            window.draw(inputText);
        }

        window.display();
    }
};

int main() {
    Platform platform;
    platform.run();
    return 0;
}