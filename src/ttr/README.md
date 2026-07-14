# TTr (Tamizhi Transpiler) 🔄🐧

**TTr** is a fast, native source-to-source compiler (Transpiler) written entirely in **C**. It translates Python code into **[Tamizhi](https://github.com/your-username/tamizhi)** (`.tz`) syntax. 

By leveraging TTr, developers can write logic in Python, convert it to Tamizhi, and compile it down to highly optimized, standalone native Linux binaries via LLVM/Clang. No Python runtime is required to execute the final output!

## 🚀 Why TTr?
- **Zero Dependency Executables:** Convert Python scripts into standalone Linux binaries.
- **Native Performance:** Bypasses the Python interpreter, allowing your logic to run at near-C speeds on Linux environments.
- **Democratizing Systems Programming:** Allows Python developers to easily build native CLI tools for Linux without needing to learn complex systems languages.

## ⚙️ Architecture Under the Hood
TTr is built using standard compiler design architecture for maximum efficiency and scalability:
* **Lexer:** Tokenizes the Python source code.
* **Parser:** Generates an Abstract Syntax Tree (AST) to understand the logic.
* **CodeGen:** Maps the logic to Tamizhi syntax (`call`, `func`, etc.) and generates the `.tz` file.

## 🛠️ Building & Usage (Coming Soon)
```bash
# Clone the repository
git clone [https://github.com/your-username/TTr.git](https://github.com/your-username/TTr.git)
cd TTr

# Compile the transpiler
gcc -o ttr src/*.c

# Translate a Python file to Tamizhi
./ttr app.py 
# Output: app.tz is generated automatically!
