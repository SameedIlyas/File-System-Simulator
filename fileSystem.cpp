#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include <memory>
#include <stack>

class File {
public:
    std::string name;
    std::string content;

    File(std::string name) : name(name), content("") {}

    void write(const std::string& text, bool append = true) {
        if (append) {
            content += text;
        } else {
            content = text;
        }
    }

    void write_at(int pos, const std::string& text) {
        if (pos > content.length()) {
            content.resize(pos);
        }
        for (int i = 0; i < text.size(); ++i) {
            if (pos + i < content.size()) {
                content[pos + i] = text[i];
            } else {
                content += text[i];
            }
        }
    }

    std::string read() {
        return content;
    }

    std::string read_from(int start, int numChars) {
        if (start >= content.length()) return "";
        return content.substr(start, numChars);
    }
};

class Directory {
public:
    std::string name;
    std::map<std::string, std::unique_ptr<File>> files;
    std::map<std::string, std::unique_ptr<Directory>> directories;

    Directory(std::string name) : name(name) {}
};

class FileSystem {
private:
    std::unique_ptr<Directory> root;
    Directory* currentDir;
    std::stack<Directory*> dirStack; // For navigation

    void serialize(std::ostream& out, Directory* dir, const std::string& path) {
        for (auto& filePair : dir->files) {
            out << "F " << path + "/" + filePair.first << " " << filePair.second->content.size() << " " << filePair.second->content << "\n";
        }
        for (auto& dirPair : dir->directories) {
            out << "D " << path + "/" + dirPair.first << "\n";
            serialize(out, dirPair.second.get(), path + "/" + dirPair.first);
        }
    }

    void deserialize(std::istream& in) {
        std::string line;
        while (getline(in, line)) {
            std::istringstream iss(line);
            char type;
            std::string path, content;
            size_t contentSize;
            iss >> type >> path;
            switch (type) {
                case 'F':
                    iss >> contentSize;
                    content.resize(contentSize);
                    iss.read(&content[0], contentSize); // Directly read into the string
                    createFileAtPath(path, content);
                    break;
                case 'D':
                    mkdirAtPath(path);
                    break;
            }
        }
    }

    void createFileAtPath(const std::string& fullPath, const std::string& content) {
        size_t lastSlashPos = fullPath.find_last_of('/');
        std::string path = fullPath.substr(0, lastSlashPos);
        std::string fileName = fullPath.substr(lastSlashPos + 1);
        Directory* dir = navigateToPath(path);
        if (dir) {
            if (dir->files.find(fileName) == dir->files.end()) {
                dir->files[fileName] = std::make_unique<File>(fileName);
                dir->files[fileName]->content = content;
            }
        }
    }

    void mkdirAtPath(const std::string& fullPath) {
        navigateToPath(fullPath, true);
    }

    Directory* navigateToPath(const std::string& path, bool create = false) {
        std::istringstream iss(path);
        std::string part;
        Directory* dir = root.get();
        while (getline(iss, part, '/')) {
            if (part.empty()) continue;
            if (dir->directories.find(part) == dir->directories.end()) {
                if (create) {
                    dir->directories[part] = std::make_unique<Directory>(part);
                } else {
                    return nullptr; // Path does not exist
                }
            }
            dir = dir->directories[part].get();
        }
        return dir;
    }

public:
    FileSystem() {
        root = std::make_unique<Directory>("root");
        currentDir = root.get();
    }

    ~FileSystem() {
        saveToFile("filesystem.dat"); // Save state on destruction
    }

    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            deserialize(file);
            file.close();
        }
    }

    void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
            serialize(file, root.get(), "");
            file.close();
        }
    }
    
    void create(const std::string& fName) {
        if (currentDir->files.find(fName) == currentDir->files.end()) {
            currentDir->files[fName] = std::make_unique<File>(fName);
            std::cout << "File created: " << fName << std::endl;
            saveToFile("filesystem.dat");
        } else {
            std::cerr << "Error: File already exists." << std::endl;
        }
    }

    void remove(const std::string& fName) {
        if (currentDir->files.erase(fName) == 0) {
            std::cerr << "Error: File not found." << std::endl;
        } else {
            std::cout << "File deleted: " << fName << std::endl;
            saveToFile("filesystem.dat");
        }
    }

    void mkdir(const std::string& dirName) {
        if (currentDir->directories.find(dirName) == currentDir->directories.end()) {
            currentDir->directories[dirName] = std::make_unique<Directory>(dirName);
            std::cout << "Directory created: " << dirName << std::endl;
            saveToFile("filesystem.dat");
        } else {
            std::cerr << "Error: Directory already exists." << std::endl;
        }
    }

void chDir(const std::string& dirName) {
    if (dirName == "..") {
        // Special case for moving up to parent directory if possible
        if (!dirStack.empty()) {
            currentDir = dirStack.top();  // Go back to the previous directory
            dirStack.pop();
            std::cout << "Changed directory to: " << currentDir->name << std::endl;
        } else {
            std::cerr << "Error: No parent directory." << std::endl;
        }
    } else {
        auto it = currentDir->directories.find(dirName);
        if (it != currentDir->directories.end()) {
            dirStack.push(currentDir);  // Save current directory to stack
            currentDir = it->second.get();  // Change to the new directory
            std::cout << "Changed directory to: " << dirName << std::endl;
        } else {
            std::cerr << "Error: Directory does not exist." << std::endl;
        }
    }
}

  void move(const std::string& sourcePath, std::string& targetPath) {
    size_t lastSlashSrc = sourcePath.find_last_of('/');
    std::string sourceDirPath = sourcePath.substr(0, lastSlashSrc);
    std::string sourceFileName = sourcePath.substr(lastSlashSrc + 1);

    size_t lastSlashTgt = targetPath.find_last_of('/');
    std::string targetDirPath = targetPath.substr(0, lastSlashTgt);
    std::string targetFileName = targetPath.substr(lastSlashTgt + 1);

    // Navigate to the source directory
    Directory* sourceDir = navigateToPath(sourceDirPath);
    if (!sourceDir) {
        std::cerr << "Error: Source directory does not exist." << std::endl;
        return;
    }

    // Navigate to the target directory, create if not exists
    Directory* targetDir = navigateToPath(targetDirPath, true);
    if (!targetDir) {
        std::cerr << "Error: Target directory could not be ensured." << std::endl;
        return;
    }

    // Move the file
    auto fileIt = sourceDir->files.find(sourceFileName);
    if (fileIt == sourceDir->files.end()) {
        std::cerr << "Error: Source file does not exist." << std::endl;
        return;
    }

    // Check if target file already exists
    if (targetDir->files.find(targetFileName) != targetDir->files.end()) {
        std::cerr << "Error: Target file already exists." << std::endl;
        return;
    }

    // Move the file to the new directory
    targetDir->files[targetFileName] = std::move(fileIt->second);
    sourceDir->files.erase(fileIt);
    std::cout << "File moved from " << sourcePath << " to " << targetPath << std::endl;
}

    File* open(const std::string& fName, const std::string& mode) {
        auto it = currentDir->files.find(fName);
        if (it != currentDir->files.end()) {
            return it->second.get();
        } else {
            std::cerr << "Error: File does not exist." << std::endl;
            return nullptr;
        }
    }

    // No actual implementation needed for 'close' in this context
    void close(File* file) {
        // Intentionally empty
    }

    // Implementations of create, delete, mkdir, chDir, move, open, close, write, read
    
        void listDirectory() {
        if (currentDir == nullptr) {
            std::cout << "No current directory selected." << std::endl;
            return;
        }

        std::cout << "Current Directory: " << currentDir->name << std::endl;
        std::cout << "Directories:" << std::endl;
        for (const auto& dir : currentDir->directories) {
            std::cout << "  [D] " << dir.first << std::endl;
        }

        std::cout << "Files:" << std::endl;
        for (const auto& file : currentDir->files) {
            std::cout << "  [F] " << file.first << std::endl;
        }
    }
};

int main() {
    FileSystem fs;
    fs.loadFromFile("filesystem.dat"); // Load existing filesystem state

    std::string input;
    int choice;

    while (true) {
        std::cout << "\n--- File System Menu ---\n";
        std::cout << "1. List Directory Contents\n";
        std::cout << "2. Create New File\n";
        std::cout << "3. Delete File\n";
        std::cout << "4. Create New Directory\n";
        std::cout << "5. Change Directory\n";
        std::cout << "6. Move File\n";
        std::cout << "7. Write to File\n";
        std::cout << "8. Read from File\n";
        std::cout << "9. Write at fixed position in file\n";
        std::cout << "10. Read fixed number of characters from file\n";
        std::cout << "11. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Clear the input buffer

        switch (choice) {
            case 1:  // List contents
                fs.listDirectory();
                break;
            case 2: {  // Create file
                std::string fileName;
                std::cout << "Enter filename to create: ";
                getline(std::cin, fileName);
                fs.create(fileName);
                break;
            }
            case 3: {  // Delete file
                std::string fileName;
                std::cout << "Enter filename to delete: ";
                getline(std::cin, fileName);
                fs.remove(fileName);
                break;
            }
            case 4: {  // Create directory
                std::string dirName;
                std::cout << "Enter directory name to create: ";
                getline(std::cin, dirName);
                fs.mkdir(dirName);
                break;
            }
            case 5: {  // Change directory
                std::string dirName;
                std::cout << "Enter directory name to change to: ";
                getline(std::cin, dirName);
                fs.chDir(dirName);
                break;
            }
            case 6: {  // Move file
                std::string sourcePath, targetPath;
                std::cout << "Enter the full path of the source file: ";
                getline(std::cin, sourcePath);
                std::cout << "Enter the full path of the target file: ";
                getline(std::cin, targetPath);
                fs.move(sourcePath, targetPath);
                break;
            }
            case 7: {  // Write to file
                std::string fileName, text;
                std::cout << "Enter filename to write to: ";
                getline(std::cin, fileName);
                std::cout << "Enter text to write: ";
                getline(std::cin, text);
                File* file = fs.open(fileName, "write");
                if (file) {
                    file->write(text);
                    fs.close(file);
                }
                break;
            }
            case 8: {  // Read from file
                std::string fileName;
                std::cout << "Enter filename to read from: ";
                getline(std::cin, fileName);
                File* file = fs.open(fileName, "read");
                if (file) {
                    std::cout << "File content:\n" << file->read() << std::endl;
                    fs.close(file);
                }
                break;
            }
            case 9: {  // Write at fixed position in file
                std::string fileName;
                int position;
                std::string text;
                std::cout << "Enter filename to write to: ";
                getline(std::cin, fileName);
                std::cout << "Enter the position to start writing at: ";
                std::cin >> position;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the buffer
                std::cout << "Enter text to write: ";
                getline(std::cin, text);
                File* file = fs.open(fileName, "write");
                if (file) {
                    file->write_at(position, text);
                    fs.close(file);
                }
                break;
            }
            case 10: {  // Read fixed number of characters from file
                std::string fileName;
                int start, numChars;
                std::cout << "Enter filename to read from: ";
                getline(std::cin, fileName);
                std::cout << "Enter start position: ";
                std::cin >> start;
                std::cout << "Enter number of characters to read: ";
                std::cin >> numChars;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the buffer
                File* file = fs.open(fileName, "read");
                if (file) {
                    std::cout << "File content:\n" << file->read_from(start, numChars) << std::endl;
                    fs.close(file);
                }
                break;
            }
            case 11:  // Exit
                fs.saveToFile("filesystem.dat");  // Save current state
                return 0;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }

    return 0;
}
