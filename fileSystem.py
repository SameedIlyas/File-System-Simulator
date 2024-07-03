import os
import json

class File:
    def __init__(self, name):
        self.name = name
        self.content = ""

    def write(self, text, append=True):
        if append:
            self.content += text
        else:
            self.content = text

    def write_at(self, pos, text):
        if pos > len(self.content):
            self.content = self.content.ljust(pos)
        for i in range(len(text)):
            if pos + i < len(self.content):
                self.content = self.content[:pos + i] + text[i] + self.content[pos + i + 1:]
            else:
                self.content += text[i]

    def read(self):
        return self.content

    def read_from(self, start, num_chars):
        if start >= len(self.content):
            return ""
        return self.content[start:start + num_chars]

class Directory:
    def __init__(self, name):
        self.name = name
        self.files = {}
        self.directories = {}

class FileSystem:
    def __init__(self):
        self.root = Directory("root")
        self.current_dir = self.root
        self.dir_stack = []

    def serialize(self, dir, path):
        data = []
        for file_name, file_obj in dir.files.items():
            data.append({
                "type": "F",
                "path": path + "/" + file_name,
                "content": file_obj.content
            })
        for dir_name, subdir in dir.directories.items():
            data.append({
                "type": "D",
                "path": path + "/" + dir_name
            })
            data.extend(self.serialize(subdir, path + "/" + dir_name))
        return data

    def deserialize(self, data):
        for entry in data:
            if entry["type"] == "F":
                self.create_file_at_path(entry["path"], entry["content"])
            elif entry["type"] == "D":
                self.mkdir_at_path(entry["path"])

    def create_file_at_path(self, full_path, content):
        parts = full_path.strip("/").split("/")
        dir_path = "/".join(parts[:-1])
        file_name = parts[-1]
        dir = self.navigate_to_path(dir_path)
        if dir:
            if file_name not in dir.files:
                dir.files[file_name] = File(file_name)
                dir.files[file_name].content = content

    def mkdir_at_path(self, full_path):
        self.navigate_to_path(full_path, create=True)

    def navigate_to_path(self, path, create=False):
        parts = path.strip("/").split("/")
        dir = self.root
        for part in parts:
            if not part:
                continue
            if part not in dir.directories:
                if create:
                    dir.directories[part] = Directory(part)
                else:
                    return None
            dir = dir.directories[part]
        return dir

    def load_from_file(self, filename):
        try:
            with open(filename, "r") as file:
                data = json.load(file)
                self.deserialize(data)
        except FileNotFoundError:
            pass

    def save_to_file(self, filename):
        with open(filename, "w") as file:
            data = self.serialize(self.root, "")
            json.dump(data, file)

    def create(self, f_name):
        if f_name not in self.current_dir.files:
            self.current_dir.files[f_name] = File(f_name)
            print(f"File created: {f_name}")
            self.save_to_file("filesystem.json")
        else:
            print("Error: File already exists.")

    def remove(self, f_name):
        if f_name in self.current_dir.files:
            del self.current_dir.files[f_name]
            print(f"File deleted: {f_name}")
            self.save_to_file("filesystem.json")
        else:
            print("Error: File not found.")

    def mkdir(self, dir_name):
        if dir_name not in self.current_dir.directories:
            self.current_dir.directories[dir_name] = Directory(dir_name)
            print(f"Directory created: {dir_name}")
            self.save_to_file("filesystem.json")
        else:
            print("Error: Directory already exists.")

    def chdir(self, dir_name):
        if dir_name == "..":
            if self.dir_stack:
                self.current_dir = self.dir_stack.pop()
                print(f"Changed directory to: {self.current_dir.name}")
            else:
                print("Error: No parent directory.")
        else:
            if dir_name in self.current_dir.directories:
                self.dir_stack.append(self.current_dir)
                self.current_dir = self.current_dir.directories[dir_name]
                print(f"Changed directory to: {dir_name}")
            else:
                print("Error: Directory does not exist.")

    def move(self, source_path, target_path):
        src_parts = source_path.strip("/").split("/")
        tgt_parts = target_path.strip("/").split("/")

        src_dir_path = "/".join(src_parts[:-1])
        src_file_name = src_parts[-1]
        tgt_dir_path = "/".join(tgt_parts[:-1])
        tgt_file_name = tgt_parts[-1]

        src_dir = self.navigate_to_path(src_dir_path)
        if not src_dir:
            print("Error: Source directory does not exist.")
            return

        tgt_dir = self.navigate_to_path(tgt_dir_path, create=True)
        if not tgt_dir:
            print("Error: Target directory could not be ensured.")
            return

        if src_file_name in src_dir.files:
            if tgt_file_name in tgt_dir.files:
                print("Error: Target file already exists.")
                return

            tgt_dir.files[tgt_file_name] = src_dir.files.pop(src_file_name)
            print(f"File moved from {source_path} to {target_path}")
        else:
            print("Error: Source file does not exist.")

    def open(self, f_name, mode):
        if f_name in self.current_dir.files:
            return self.current_dir.files[f_name]
        else:
            print("Error: File does not exist.")
            return None

    def close(self, file):
        pass  # No action needed

    def list_directory(self):
        print(f"Current Directory: {self.current_dir.name}")
        print("Directories:")
        for dir_name in self.current_dir.directories:
            print(f"  [D] {dir_name}")
        print("Files:")
        for file_name in self.current_dir.files:
            print(f"  [F] {file_name}")

def main():
    fs = FileSystem()
    fs.load_from_file("filesystem.json")  # Load existing filesystem state

    while True:
        print("\n--- File System Menu ---")
        print("1. List Directory Contents")
        print("2. Create New File")
        print("3. Delete File")
        print("4. Create New Directory")
        print("5. Change Directory")
        print("6. Move File")
        print("7. Write to File")
        print("8. Read from File")
        print("9. Write at fixed position in file")
        print("10. Read fixed number of characters from file")
        print("11. Exit")
        choice = input("Enter your choice: ")

        if choice == "1":
            fs.list_directory()
        elif choice == "2":
            file_name = input("Enter filename to create: ")
            fs.create(file_name)
        elif choice == "3":
            file_name = input("Enter filename to delete: ")
            fs.remove(file_name)
        elif choice == "4":
            dir_name = input("Enter directory name to create: ")
            fs.mkdir(dir_name)
        elif choice == "5":
            dir_name = input("Enter directory name to change to: ")
            fs.chdir(dir_name)
        elif choice == "6":
            source_path = input("Enter the full path of the source file: ")
            target_path = input("Enter the full path of the target file: ")
            fs.move(source_path, target_path)
        elif choice == "7":
            file_name = input("Enter filename to write to: ")
            text = input("Enter text to write: ")
            file = fs.open(file_name, "write")
            if file:
                file.write(text)
                fs.close(file)
        elif choice == "8":
            file_name = input("Enter filename to read from: ")
            file = fs.open(file_name, "read")
            if file:
                print("File content:\n" + file.read())
                fs.close(file)
        elif choice == "9":
            file_name = input("Enter filename to write to: ")
            position = int(input("Enter the position to start writing at: "))
            text = input("Enter text to write: ")
            file = fs.open(file_name, "write")
            if file:
                file.write_at(position, text)
                fs.close(file)
        elif choice == "10":
            file_name = input("Enter filename to read from: ")
            start = int(input("Enter start position: "))
            num_chars = int(input("Enter number of characters to read: "))
            file = fs.open(file_name, "read")
            if file:
                print("File content:\n" + file.read_from(start, num_chars))
                fs.close(file)
        elif choice == "11":
            fs.save_to_file("filesystem.json")  # Save current state
            break
        else:
            print("Invalid choice. Please try again.")

if __name__ == "__main__":
    main()
