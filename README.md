# 💸 Money Tracker (with GUI)

**Author**: Almohtady Bellah  
**License**: MIT   

---

## 🔍 Overview

**Money Tracker** is a lightweight **finance manager** written in C++.  
It helps users track transactions, manage personal loans (borrowers/lenders), and write short notes.

Ideal for:
- Personal finance tracking
- Practicing OOP and file handling in C++
- Building toward a full-featured GUI app

---

## 📜 Current Features

- **View Data**: Show all saved records  
- **Make A Transaction**: Add income/expense entries  
- **Write A Short Note**: Quick notes tied to sessions  
- **Manage Borrowers/Lenders**: Add or update people you owe or lend to  
- **RESET**: Wipe all data and exit (use carefully)  

---

## 🛠️ How to Build (CMake, Windows Only)

This project uses Windows APIs (windows.h) for encryption, so it can only be compiled on Windows.
You can build it either with MSVC (Visual Studio) or MinGW.

### 🔹 Option 1: Build with MSVC (recommended)

1. Clone the repo with submodules:
```cmd
git clone --recursive https://github.com/almohtadyg1/Money-Tracker-with-gui.git
cd Money-Tracker-with-gui
```
2. Configure & build with CMake:
```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The executable will be in:
`build/Release/MoneyTracker.exe`

### 🔹 Option 2: Build with MinGW

1. Clone the repo with submodules:
```cmd
git clone --recursive https://github.com/almohtadyg1/Money-Tracker-with-gui.git
cd Money-Tracker-with-gui
```
2. Configure & build with CMake:
```cmd
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
```

The executable will be in:
`build/MoneyTracker.exe`

> 💡 Requires a C++20 compatible compiler (MSVC 2019+/MinGW-w64).  
> On GitHub Actions, the project is automatically built with MSVC.

---

## 🤝 Contributing

Feel free to open issues or pull requests if you'd like to contribute ideas, fixes, or improvements.

---

## 🧠 Inspiration

Built as a learning project and stepping stone to larger, GUI-based tools.  
The goal is to evolve this into a powerful yet lightweight personal finance application.


---

## 📬 Contact

Check out my website: [almohtadyg1.pythonanywhere.com](https://almohtadyg1.pythonanywhere.com/)

---

## 🪪 License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT).
