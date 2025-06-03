
#include <iostream>  
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <limits>
#include<regex>
#include "json.hpp"
//no vector used 
//elie fakhoury
using json = nlohmann::json;
using namespace std;

struct User {
    char firstName[100];  
    char lastName[100];   
    char email[100];
    char phone[20];       
    char password[100];
    char role[10];     
};

struct Donation {
    int donationId;
    int charityId;
    int donorId;
    double amount;
    string donationDateTime;
    string donorFullName;
    string message;
};

struct Charity {
    int id;
    string name;
   string description;
    double targetAmount;
    double currentAmount;
};




void saveDonations(const json& donations) {
    // Open the file in write mode
    std::ofstream file("donations.json");
    
    if (!file.is_open()) {
        cout<< "Error opening donations file for writing.\n";
        return;
    }

    // Write the donations data to the file
    file << donations.dump(4);  // '4' is for pretty printing with 4 spaces indentation

    file.close();
}

string getCurrentDateTime() {
    time_t now = time(0);
    tm* localtm = localtime(&now);
    stringstream ss;
    ss << put_time(localtm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string toLowerCase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

size_t hashPassword(const string& password) {
    return hash<string>{}(password);
}

json loadFile(const string& filename) {
    ifstream inFile(filename);
    json data = json::array();
    if (inFile.is_open()) {
        inFile >> data;
    }
    return data;
}

void saveFile(const string& filename, const json& data) {
    ofstream outFile(filename);
    outFile << setw(4) << data << endl;
}
bool isValidPassword(const string& password) {
    if (password.length() < 8)
        return false;

    bool hasUppercase = false;
    bool hasSpecialChar = false;

    for (char c : password) {
        if (isupper(c)) {
            hasUppercase = true;
        }
        if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) ||
            (c >= 91 && c <= 96) || (c >= 123 && c <= 126)) {
            hasSpecialChar = true;
        }
    }

    return hasUppercase && hasSpecialChar;
}
bool isValidDomain(const string& email) {
    size_t atPos = email.find('@');
    if (atPos == string::npos || atPos == email.length() - 1)
        return false;

    string domain = email.substr(atPos + 1);  // Part after '@'

    if (domain.empty() || domain.front() == '.' || domain.back() == '.')
        return false;

    string segment;
    for (size_t i = 0; i < domain.length(); ++i) {
        char c = domain[i];

        if (isalnum(c)) {
            segment += c;
        } else if (c == '.') {
            if (segment.empty()) return false;  // ".." or ".start"
            segment.clear(); // Start a new segment
        } else {
            return false; // Invalid character
        }
    }

    return !segment.empty(); // Last segment must not be empty
}

bool isValidEmailFormat(const string& email) {
    size_t atPos = email.find('@');
    if (atPos == string::npos || atPos == 0 || atPos == email.size() - 1) {
        return false;  // '@' is missing or at the beginning/end
    }

    size_t dotPos = email.find('.', atPos);
    return dotPos != string::npos && dotPos != email.size() - 1;  // '.' should exist after '@'
}
bool isValidPhoneNumber(const string& phone) {
    // Check if phone has exactly 8 digits
    if (phone.length() != 8 || any_of(phone.begin(), phone.end(), [](char c) { return !isdigit(c); })) {
        return false;
    }

    // Check if it starts with valid Lebanese prefixes: 01, 03, 76, 71, 81, 91
    string prefix = phone.substr(0, 2);
    if (prefix != "01" && prefix != "03" && prefix != "76" && prefix != "71" && prefix != "81" && prefix != "91"&& prefix != "05"&& prefix != "06"&& prefix != "08"&& prefix != "70" && prefix != "79") 
        {
        return false;
    }
    return true;
}



void registerUser() {
    json users = loadFile("users.json");

    User newUser;
    std::string input;

    // First name
    cout << "Enter first name: ";
    getline(cin, input);
    strncpy(newUser.firstName, input.c_str(), sizeof(newUser.firstName));
    newUser.firstName[sizeof(newUser.firstName) - 1] = '\0';

    // Last name
    cout << "Enter last name: ";
    getline(cin, input);
    strncpy(newUser.lastName, input.c_str(), sizeof(newUser.lastName));
    newUser.lastName[sizeof(newUser.lastName) - 1] = '\0';

    // Email validation and input loop
    bool exists;
    do {
        cout << "Enter email: ";
        getline(cin, input);
        input = toLowerCase(input);

        if (!isValidEmailFormat(input) || !isValidDomain(input)) {
            cout << "Invalid email format or unsupported domain. Please try again.\n";
            exists = true;
            continue;
        }

        exists = false;
        for (auto& u : users) {
            if (u["email"] == input) {
                exists = true;
                break;
            }
        }
        if (exists) {
            cout << "Email already registered. Please enter a different email.\n";
        }
    } while (exists);

    strncpy(newUser.email, input.c_str(), sizeof(newUser.email));
    newUser.email[sizeof(newUser.email) - 1] = '\0';

    // Phone validation and input loop
    do {
        cout << "Enter phone number (8 digits starting with allowed prefixes): ";
        getline(cin, input);

        if (!isValidPhoneNumber(input)) {
            cout << "Invalid phone number. Please try again.\n";
            continue;
        }

        bool phoneExists = false;
        for (auto& u : users) {
            if (u["phone"] == input) {
                phoneExists = true;
                break;
            }
        }
        if (phoneExists) {
            cout << "Phone number already in use. Please enter a different number.\n";
            continue;
        }
        break;
    } while (true);

    strncpy(newUser.phone, input.c_str(), sizeof(newUser.phone));
    newUser.phone[sizeof(newUser.phone) - 1] = '\0';

    // Password input & validation
    string password, confirmPassword;
    do {
        cout << "Enter password (min 8 chars, uppercase, special char): ";
        getline(cin, password);
        if (!isValidPassword(password)) {
            cout << "Invalid password.\n";
            continue;
        }

        cout << "Re-enter password to confirm: ";
        getline(cin, confirmPassword);
        if (password != confirmPassword) {
            cout << "Passwords do not match.\n";
            continue;
        }
        break;
    } while (true);

    // Store hashed password as string (convert size_t hash to string)
    size_t hashed = hashPassword(password);
    snprintf(newUser.password, sizeof(newUser.password), "%zu", hashed);

    // Role input and validation
    cout << "Enter role (admin/donor): ";
    getline(cin, input);
    input = toLowerCase(input);

    if (input == "admin") {
        string auth;
        cout << "Enter admin authentication password: ";
        getline(cin, auth);
        if (auth != "admin23@") {
            cout << "Invalid admin authentication.\n";
            return;
        }
    } else if (input != "donor") {
        cout << "Invalid role.\n";
        return;
    }

    strncpy(newUser.role, input.c_str(), sizeof(newUser.role));
    newUser.role[sizeof(newUser.role) - 1] = '\0';

    // Assign new user ID
    int userId = users.empty() ? 1 : users.back()["id"].get<int>() + 1;

    // Add new user to JSON
    users.push_back({
        {"id", userId},
        {"firstName", newUser.firstName},
        {"lastName", newUser.lastName},
        {"email", newUser.email},
        {"phone", newUser.phone},
        {"password", newUser.password},
        {"role", newUser.role}
    });

    saveFile("users.json", users);
    cout << "Registration successful!\n";
}
json loginUser() {
    json users = loadFile("users.json");  // Load users from JSON file

    string email, password;
    cout << "Enter email: ";
    getline(cin, email);
    email = toLowerCase(email);  // Normalize the email to lowercase

    // Check if the email exists in the users list
    auto userIt = find_if(users.begin(), users.end(), [&email](const json& user) {
        return user["email"] == email;
    });

    if (userIt != users.end()) {
        cout << "Enter password: ";
        getline(cin, password);

        size_t hashed = hashPassword(password);

        // Check if the password matches
        if ((*userIt)["password"] == hashed) {
            cout << "Login successful! Welcome " << (*userIt)["role"] << ".\n";
//Use the struct to store the user data
            User u;
            strcpy(u.firstName, (*userIt)["firstName"].get<string>().c_str());
            strcpy(u.lastName,  (*userIt)["lastName"].get<string>().c_str());
            strcpy(u.email,     (*userIt)["email"].get<string>().c_str());
            strcpy(u.phone,     (*userIt)["phone"].get<string>().c_str());
            strcpy(u.role,      (*userIt)["role"].get<string>().c_str());
            // No need to copy password here, but if you want:
            // strcpy(u.password, password.c_str());

            // Return json (original format as you asked)
            return *userIt;
        } else {
            cout << "Incorrect password. Please try again.\n";
        }
    } else {
        cout << "No account found with this email. Please try again or register.\n";
    }

    return json{};  // Return an empty json object if login fails
}

// Charities
json loadCharities() {
    return loadFile("charities.json");
}


   void saveCharities(const json& charities) {
    ofstream file("charities.json");
    if (file.is_open()) {
        file << setw(4) << charities;
        file.close();
    } else {
        cerr << "Error: Could not open charities.json for writing.\n";
    }
}




// Donations
json loadDonations() {
    return loadFile("donations.json");
}
Donation jsonToDonation(const json& j) {
    Donation d;
    d.donationId = j["donationId"];
    d.charityId = j["charityId"];
    d.donorId = j["donorId"];

    std::string amt = j["amount"];
    d.amount = std::stod(amt.substr(1));  // remove '$' sign and convert

    d.donationDateTime = j["donationDateTime"];
    d.donorFullName = j["donorFullName"];
    d.message = j["message"];
    return d;
}


void donate(json& charities, const json& user, int donorId) {
    if (charities.empty()) {
        cout << "No charities available to donate.\n";
        return;
    }

    // Display charities
    for (size_t i = 0; i < charities.size(); i++) {
        cout << "ID: " << charities[i]["id"]
             << " | Name: " << charities[i]["name"]
             << " | Target: $" << charities[i]["targetAmount"]
             << " | Collected: $" << charities[i]["currentAmount"] << endl;
    }

    int id;
    cout << "Enter Charity ID to donate to: ";
    cin >> id;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid charity ID input.\n";
        return;
    }

    // Find the charity by id
    size_t charityIndex = charities.size(); // out of range initially
    for (size_t i = 0; i < charities.size(); i++) {
        if (charities[i]["id"] == id) {
            charityIndex = i;
            break;
        }
    }

    if (charityIndex == charities.size()) {
        cout << "Charity not found.\n";
        return;
    }

    double amount;
    cout << "Enter amount to donate: ";
    cin >> amount;
    if (cin.fail() || amount <= 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid donation amount.\n";
        return;
    }

    double current = charities[charityIndex]["currentAmount"].get<double>();
    double target = charities[charityIndex]["targetAmount"].get<double>();

    if (current + amount > target) {
        cout << "Donation exceeds the target amount. Only $" << (target - current) << " is needed.\n";
        return;
    }

    charities[charityIndex]["currentAmount"] = current + amount;
    saveCharities(charities);

    json donations = loadDonations();

    // Find max donationId
    int maxId = 0;
    for (size_t i = 0; i < donations.size(); i++) {
        int currId = donations[i]["donationId"].get<int>();
        if (currId > maxId) maxId = currId;
    }

    // Compose donor full name
    string donorFullName = user["firstName"].get<string>() + " " + user["lastName"].get<string>();

    // Optional message
    string message;
    cout << "Would you like to add a message to this donation? (y/n): ";
    char choice;
    cin >> choice;
    cin.ignore();  // clear newline

    if (choice == 'y' || choice == 'Y') {
        cout << "Enter your message: ";
        getline(cin, message);
    }

    donations.push_back({
        {"donationId", maxId + 1},
        {"charityId", id},
        {"donorId", donorId},
        {"amount", "$" + to_string(amount)},
        {"donationDateTime", getCurrentDateTime()},
        {"donorFullName", donorFullName},
        {"message", message}
    });

    saveDonations(donations);
    cout << "Thank you for your donation!" << (message.empty() ? "" : " Your message has been added.\n");
}

json donationToJson(const Donation& d) {
    return json{
        {"donationId", d.donationId},
        {"charityId", d.charityId},
        {"donorId", d.donorId},
        {"amount", "$" + std::to_string(d.amount)},
        {"donationDateTime", d.donationDateTime},
        {"donorFullName", d.donorFullName},
        {"message", d.message}
    };
}

Charity jsonToCharity(const json& j) {//charity is a structure function
    Charity c;
    c.id = j["id"];
    c.name = j["name"];
    c.description = j["description"];
    c.targetAmount = j["targetAmount"];
    c.currentAmount = j["currentAmount"];
    return c;
}

json charityToJson(const Charity& c) {
    return json{
        {"id", c.id},
        {"name", c.name},
        {"description", c.description},
        {"targetAmount", c.targetAmount},
        {"currentAmount", c.currentAmount}
    };
}


void cancelDonation(const json& user) {
    json donationsJson = loadDonations();
    json charitiesJson = loadCharities();
    int donorId = user["id"];

    // Convert JSON to arrays of structs (dynamic allocation)
    int donationsCount = donationsJson.size();
    Donation* donations = new Donation[donationsCount];
    for (int i = 0; i < donationsCount; i++) {
        donations[i] = jsonToDonation(donationsJson[i]);
    }

    int charitiesCount = charitiesJson.size();
    Charity* charities = new Charity[charitiesCount];
    for (int i = 0; i < charitiesCount; i++) {
        charities[i] = jsonToCharity(charitiesJson[i]);
    }

    // Display donor's donations
    bool hasDonations = false;
    cout << "\nYour Donations:\n";
    for (int i = 0; i < donationsCount; i++) {
        if (donations[i].donorId == donorId) {
            hasDonations = true;
            cout << "Donation ID: " << donations[i].donationId
                 << " | Charity ID: " << donations[i].charityId
                 << " | Amount: $" << donations[i].amount
                 << " | Date: " << donations[i].donationDateTime << endl;
        }
    }

    if (!hasDonations) {
        cout << "You have not made any donations.\n";
        delete[] donations;
        delete[] charities;
        return;
    }

    int cancelId;
    cout << "Enter the Donation ID you want to cancel: ";
    cin >> cancelId;
    cin.ignore();

    // Find donation index
    int cancelIndex = -1;
    for (int i = 0; i < donationsCount; i++) {
        if (donations[i].donationId == cancelId && donations[i].donorId == donorId) {
            cancelIndex = i;
            break;
        }
    }

    if (cancelIndex == -1) {
        cout << "Donation not found or does not belong to you.\n";
        delete[] donations;
        delete[] charities;
        return;
    }

    // Update charity currentAmount
    int charityId = donations[cancelIndex].charityId;
    double amount = donations[cancelIndex].amount;

    for (int i = 0; i < charitiesCount; i++) {
        if (charities[i].id == charityId) {
            charities[i].currentAmount -= amount;
            if (charities[i].currentAmount < 0) charities[i].currentAmount = 0;
            break;
        }
    }

    // Remove the donation by shifting elements left
    for (int i = cancelIndex; i < donationsCount - 1; i++) {
        donations[i] = donations[i + 1];
    }
    donationsCount--;

    // Convert arrays back to JSON
    json updatedDonationsJson = json::array();
    for (int i = 0; i < donationsCount; i++) {
        updatedDonationsJson.push_back(donationToJson(donations[i]));
    }

    json updatedCharitiesJson = json::array();
    for (int i = 0; i < charitiesCount; i++) {
        updatedCharitiesJson.push_back(charityToJson(charities[i]));
    }

    // Save back
    saveDonations(updatedDonationsJson);
    saveCharities(updatedCharitiesJson);

    cout << "Donation canceled and amount removed from charity total.\n";

    delete[] donations;
    delete[] charities;
}

void modifyDonation(const json& user) {
    json donations = loadDonations();
    json charities = loadCharities();
    int userId = user["id"];

    cout << "\n--- Your Donations ---\n";
    bool hasDonations = false;

    for (const auto& d : donations) {
        if (d["donorId"] == userId) {
            Donation don = jsonToDonation(d);
            cout << "Donation ID: " << don.donationId
                 << " | Charity ID: " << don.charityId
                 << " | Amount: $" << fixed << setprecision(2) << don.amount
                 << " | Date/Time: " << don.donationDateTime << "\n";
            hasDonations = true;
        }
    }

    if (!hasDonations) {
        cout << "You have no donations to modify.\n";
        return;
    }

    int donationId;
    cout << "Enter the Donation ID you want to modify: ";
    cin >> donationId;

    auto it = find_if(donations.begin(), donations.end(), [&](const json& d) {
        return d["donationId"] == donationId && d["donorId"] == userId;
    });

    if (it == donations.end()) {
        cout << "Donation not found.\n";
        return;
    }

    Donation selectedDonation = jsonToDonation(*it);
    int oldCharityId = selectedDonation.charityId;
    double oldAmount = selectedDonation.amount;

    cout << "Modify Options:\n";
    cout << "1. Change amount\n";
    cout << "2. Change charity\n";
    cout << "Choose option (1 or 2): ";
    int choice;
    cin >> choice;

    if (choice == 1) {
        double newAmount;
        cout << "Enter new donation amount: ";
        cin >> newAmount;

        if (newAmount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }

        for (auto& c : charities) {
            if (c["id"] == oldCharityId) {
                c["currentAmount"] = c["currentAmount"].get<double>() - oldAmount + newAmount;
                break;
            }
        }

        ostringstream oss;
        oss << fixed << setprecision(2) << newAmount;
        (*it)["amount"] = "$" + oss.str();
        (*it)["donationDateTime"] = getCurrentDateTime();

    } else if (choice == 2) {
        cout << "\nAvailable Charities:\n";
        for (const auto& c : charities) {
            Charity ch = jsonToCharity(c);
            cout << "ID: " << ch.id << " | Name: " << ch.name << "\n";
        }

        int newCharityId;
        cout << "Enter new charity ID: ";
        cin >> newCharityId;

        auto newIt = find_if(charities.begin(), charities.end(), [&](const json& c) {
            return c["id"] == newCharityId;
        });

        if (newIt == charities.end()) {
            cout << "Invalid charity ID.\n";
            return;
        }

        for (auto& c : charities) {
            if (c["id"] == oldCharityId) {
                c["currentAmount"] = c["currentAmount"].get<double>() - oldAmount;
            }
            if (c["id"] == newCharityId) {
                c["currentAmount"] = c["currentAmount"].get<double>() + oldAmount;
            }
        }

        (*it)["charityId"] = newCharityId;
        (*it)["donationDateTime"] = getCurrentDateTime();

    } else {
        cout << "Invalid choice.\n";
        return;
    }

    saveDonations(donations);
    saveCharities(charities);
    cout << "Donation updated successfully.\n";
}


void donorMenu(const json& user) {
    json charities = loadCharities();  // Load charities initially
    json donations = loadDonations();

    while (true) {
        cout << "\n--- Donor Menu ---\n";
        cout << "1. View Charities\n";
        cout << "2. Donate\n";
        cout << "3. Cancel Donation\n";
        cout << "4. Modify Donation\n";
         
        cout << "5. Logout\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            charities = loadCharities();  // Reload the charities data
            if (charities.empty()) {
                cout << "No charities available.\n";
            } else {
                for (auto& ch : charities) {
                    cout << "ID: " << ch["id"]
                         << " | Name: " << ch["name"]
                         << " | Description: " << ch["description"]
                         << " | Target: $" << ch["targetAmount"]
                         << " | Collected: $" << ch["currentAmount"] << "\n";
                }
            }
        } else if (choice == 2) {
            donate(charities, user, user["id"]);
        } else if (choice == 3) {
            cancelDonation(user); // Cancel the user's donation
            charities = loadCharities();  // Reload to reflect changes
        } else if (choice == 4) {
            modifyDonation(user);
            charities = loadCharities();  // Reload to reflect changes
        } 
          else if(choice==5)  {
                 break;
                // Logout
        } else {
            cout << "Invalid choice.\n";
        }
    }
}



void viewAllCharities() {
    json charities = loadCharities();
    if (charities.empty()) {
        cout << "No charities found.\n";
        return;
    }
    for (const auto& ch : charities) {
        cout << "ID: " << ch["id"]
             << " | Name: " << ch["name"]
             << " | Description: " << ch["description"]
             << " | Target: $" << ch["targetAmount"]
             << " | Collected: $" << ch["currentAmount"] << "\n";
    }
}

void addCharity() {
    json charities = loadCharities();

    string name, description;
    double target;

    cout << "Enter charity name: ";
    getline(cin, name);

    cout << "Enter charity description: ";
    getline(cin, description);

    cout << "Enter target amount: ";
    cin >> target;
    cin.ignore();

    if (cin.fail() || target <= 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid target amount.\n";
        return;
    }

    int newId = charities.empty() ? 1 : charities.back()["id"].get<int>() + 1;

    // Create Charity struct
    Charity newCharity;
    newCharity.id = newId;
    newCharity.name = name;
    newCharity.description = description;
    newCharity.targetAmount = target;
    newCharity.currentAmount = 0.0;

    // Convert to json
    json charityJson;
    charityJson["id"] = newCharity.id;
    charityJson["name"] = newCharity.name;
    charityJson["description"] = newCharity.description;
    charityJson["targetAmount"] = newCharity.targetAmount;
    charityJson["currentAmount"] = newCharity.currentAmount;

    charities.push_back(charityJson);
    saveCharities(charities);
    cout << "Charity added successfully.\n";
}



void deleteCharity() {
    json charitiesJson = loadCharities();

    if (charitiesJson.empty()) {
        cout << "No charities to delete.\n";
        return;
    }

    int id;
    cout << "Enter the ID of the charity to delete: ";
    cin >> id;
    cin.ignore();

    bool found = false;
    json updatedCharities = json::array();

    for (size_t i = 0; i < charitiesJson.size(); ++i) {
        Charity c = jsonToCharity(charitiesJson[i]);//calling the structure Charity

        if (c.id == id) {
            found = true;
            continue; // skip adding this one to updated list
        }

        updatedCharities.push_back(charitiesJson[i]);
    }

    if (!found) {
        cout << "Charity not found.\n";
    } else {
        saveCharities(updatedCharities);
        cout << "Charity deleted.\n";
    }
}


void viewAllDonations() {
    json donations = loadDonations();

    if (donations.empty()) {
        cout << "No donations found.\n";
        return;
    }

    for (const auto& d : donations) {
        cout << "Donation ID: " << d["donationId"]
             << " | Charity ID: " << d["charityId"]
             << " | Donor ID: " << d["donorId"]
             << " | Amount: " << d["amount"]
             << " | Date: " << d["donationDateTime"]
             << " | Donor: " << d["donorFullName"] << endl;
    }
}
void editCharity() {
    json charities = loadCharities();

    if (charities.empty()) {
        cout << "No charities to edit.\n";
        return;
    }

    int id;
    cout << "Enter the ID of the charity to edit: ";
    cin >> id;
    cin.ignore();

    auto it = find_if(charities.begin(), charities.end(), [id](const json& ch) {
        return ch["id"] == id;
    });

    if (it == charities.end()) {
        cout << "Charity not found.\n";
        return;
    }

    string name, description;
    double target;

    cout << "Enter new charity name (leave blank to keep current): ";
    getline(cin, name);
    if (!name.empty()) (*it)["name"] = name;

    cout << "Enter new charity description (leave blank to keep current): ";
    getline(cin, description);
    if (!description.empty()) (*it)["description"] = description;

    cout << "Enter new target amount (leave blank to keep current): ";
    string targetStr;
    getline(cin, targetStr);
    if (!targetStr.empty()) {
        target = stod(targetStr);
        (*it)["targetAmount"] = target;
    }

    saveCharities(charities);
    cout << "Charity updated successfully.\n";
}

void adminMenu(const json& user) {
    while (true) {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. View All Charities\n";
        cout << "2. Add Charity\n";
        cout << "3. Delete Charity\n";
        cout << "4. Edit Charity\n";
        cout << "5.View all donations\n";  
        cout << "6. Logout\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            viewAllCharities();
        } else if (choice == 2) {
            addCharity();
        } else if (choice == 3) {
            deleteCharity();
        } else if (choice == 4) {
             editCharity();  // This is the new "Edit Charity" functionality
        } else if (choice == 5) {
          viewAllDonations();
        } 
        else if (choice == 6) {
          break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
}

int main() {
     while (true) {
        json user;
        int choice;
        cout << "\n--- Welcome to the Charity Donation System ---\n";
        cout << "Do you have an acount?\n 1-Yes \n 2-No \n";
        cin >> choice;
        cin.ignore();

        if (choice == 2) {cout<<"Welcome to the sign up page!"<<endl;
            registerUser();
        } else if (choice == 1) {
            cout<<"Welcome to the log in page!"<<endl;
            user = loginUser();
            if (!user.is_null()) {
                // Check role and call the appropriate menu
                if (user["role"] == "admin") {
                    adminMenu(user);
                } else {
                    donorMenu(user);
                }
            }
        } else if (choice == 3) {
            break;
        } else {
            cout << "Invalid choice.\n";
        }
    }

   

    return 0;
}
