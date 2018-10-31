#include <string>
#include <iostream>

using std::string;

struct Player {

  explicit Player(string line){
    std::cout << line << "\n";
    /*
    string write;
    stringstream line1(line);
    int i = 0;
    while (getline(line1, write, ';')){
        switch (i) {
            case 0:
                name = write;
                i++;
                break;
            case 1:
                surname = write;
                i++;
                break;
            case 2:
                skills[0] = write;
                i++;
                break;
            case 3:
                skills[1] = write;
                i++;
                break;
            case 4:
                skills[2] = write;
                i++;
                break;
            case 5:
                skills[3] = write;
                i++;
                break;
            case 6:
                skills[4] = write;
                i++;
                break;
            case 7:
                age = stoi(write);
                i++;
                break;
            case 8:
                height = stoi(write);
                break;

            default:
                break;
        }
        */
    }
    
};

int main() {
    string one = "one";
    int i =0;
    while (i < 10) {
        Player asdf(one);
        // addPlayer(asdf);
        ++i;
    }
}

