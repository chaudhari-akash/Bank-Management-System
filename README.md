# Banking Management System 🏦

This project simulates the core functionalities of a bank, including customer account management, transactions, and loan processing. The system prioritizes **data consistency**, **security**, and **correct handling of race conditions** through file management and locking mechanisms.

---

## ✨ Features

- Secure login system with **single session per user**
- **Role-based access**:
  - Customer
  - Bank Employee
  - Manager
  - Administrator
- Secure **file handling** and **locking** to prevent race conditions
- Supports **multiple concurrent clients** via socket programming

---

## 👩‍💻👨‍💻 User Roles & Functionalities

### 1. Customer
Customers can log in and perform various banking operations:
- 🔐 **Login System**: One session per user
- 💰 **Account Operations**: View balance, deposit, and withdraw
- 🔄 **Fund Transfers**: Transfer funds between accounts
- 🧾 **Transaction History**: View history of transactions
- 📝 **Feedback**: Submit feedback to bank
- 💸 **Loans**: Apply for loans
- 🔑 **Security**: Change password

---

### 2. Bank Employee
Bank employees manage customer accounts and loan processing:
- 🔐 **Login System**: One session per user
- 👤 **Customer Management**: Add and modify customer details
- 🏦 **Loan Management**: Process and approve/reject loan applications
- 📊 **Reporting**: View assigned loan applications and customer passbook (transaction history)
- 🔑 **Security**: Change password

---

### 3. Manager
Managers supervise employees and oversee customer accounts:
- 🔐 **Login System**: One session per user
- 🛠️ **Account Management**: Activate or deactivate customer accounts
- 👨‍💼 **Employee Management**: Assign loan tasks to employees
- 📝 **Feedback Review**: Read and respond to customer feedback
- 🔑 **Security**: Change password

---

### 4. Administrator
Admins have full control over all operations in the system:
- 🔐 **Login System**: One session per user
- 👥 **Employee Management**: Add new employees
- 🔧 **User Management**: Modify customer/employee details and roles
- 🔑 **Security**: Change password

---

## 💻 Technical Implementation

This project emphasizes **core system-level programming concepts**, including:

### Socket Programming
- The **server** handles the database and multiple **clients** concurrently
- Each client connects to the server to access account-specific features

### System Calls & File Locking
- Uses **system calls** for file operations instead of standard library functions
- Implements **file locking** to ensure data consistency and prevent **race conditions**
