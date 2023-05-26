# File System Project

This project aims to implement a file system using a tree data structure in the C programming language, integrated with Linux-based systems to enable efficient file operations. The project focuses on developing a file system that utilizes the concept of a tree data structure, providing an organized and effective approach for data storage and management.

## Key Features:
- Implementation of a tree-based file system structure using C language.
- Integration with Linux-based systems for seamless execution.
- Efficient file operations and management.
- Enhances understanding of file system concepts and improves data storage and management skills.
- Provides a user-friendly interface for performing file operations on Linux-based systems.

This project is of great importance in the field of file system management and data storage. The file system is a fundamental component of a computer system, playing a crucial role in the organized storage, access, and management of data.

## How It Works

The file system project is implemented using a tree data structure to represent the hierarchical organization of files and directories. The main components of the project are:

1. **Item**: This structure represents a file or directory item in the file system. It contains the name of the item, a pointer to its parent item, and a type indicator.
2. **File**: This structure extends the Item structure and includes an additional field called `textContent` to store the text content of the file.
3. **Directory**: This structure also extends the Item structure and includes an array of pointers called `children` to represent the child items (files or directories) within the directory. It also includes an `numChildren` field to track the number of child items.
4. **FileSystem**: This structure represents the overall file system. It includes a pointer to the root directory (`self`), a pointer to the current working directory (`currentDirectory`), and an array of pointers called `currentDirectoryPath` to store the path of the current working directory. The `currentDirectoryPathSize` field keeps track of the size of the current directory path.

The project leverages the relationships between these structures to create an efficient and organized file system. Users can perform various operations such as navigating through directories, creating new files or directories, deleting files or directories, and modifying file content.

The file system utilizes the tree data structure to maintain the hierarchy of files and directories. Each directory contains a reference to its parent directory and an array of child items. Users can traverse the file system by changing the current working directory and updating the `currentDirectory` and `currentDirectoryPath` accordingly.

File operations, such as reading from or writing to files, are performed by accessing the appropriate file items within the file system structure. The project provides an intuitive interface that allows users to interact with the file system effectively.

### The file with .c extension should be compiled and run in a Linux terminal.
