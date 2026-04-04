# Python Project Guide

This guide covers the recommended folder layout, environment configuration, dependency management, and coding guidelines for Python projects (ranging from backend APIs and microservices to CLIs and automation scripts).

The primary goals of this architecture are **modularity**, strict adherence to the **Single Responsibility Principle (SRP)**, and explicit **type safety**.

## Folder Layout

Every Python project follows a standardized `src/` layout to prevent import errors and ensure clear separation between application code, tests, and configuration:

```text
<project-name>/
├── .github/
│   └── copilot-instructions.md        # Copilot instructions file
├── .vscode/
│   └── settings.json                  # VSCode workspace settings
├── docs/
│   └── 01_specificatii-initiale.md    # Initial project specifications (from client)
├── src/                               # All application source code lives here
│   └── app/                           # Main Python package (rename to your app's name)
│       ├── __init__.py
│       ├── main.py                    # Application entry point
│       ├── api/                       # API routes/controllers (if web app)
│       ├── core/                      # Core business logic and domain models
│       ├── services/                  # Database access and external integrations
│       └── utils/                     # Pure helper functions
├── tests/                             # Unit and integration tests
│   ├── __init__.py
│   └── test_main.py
├── .env.example                       # Template for environment variables (NO SECRETS)
├── .gitattributes                     # Git attributes file
├── .gitignore                         # Git ignore file
├── requirements.txt                   # Dependency list (or pyproject.toml)
└── README.md                          # Project overview and documentation
```

## Environment & Dependency Management

- **Virtual Environments**: You **MUST** use a virtual environment (`venv`, `virtualenv`, or tools like `Poetry`/`uv`) for every project. **NEVER** install project dependencies into your global Python environment.
- **Dependency Locking**: All dependencies **MUST** be explicitly listed in `requirements.txt` (or `pyproject.toml`/`Pipfile`).
- **Environment Variables**: **NEVER** hardcode secrets, API keys, or database credentials in your Python files. **ALWAYS** use environment variables (via `os.getenv()` or libraries like `python-dotenv` / `pydantic-settings`) and document them in `.env.example`.

## Naming Conventions

Python projects **MUST** strictly adhere to [PEP 8](https://peps.python.org/pep-0008/) naming conventions.

### Files & Folders

- **Folders/Packages**: `short_lowercase` (e.g., `services`, `api`). Avoid underscores if possible.
- **Python Files (Modules)**: `lower_snake_case.py` (e.g., `user_service.py`, `database_config.py`).

### Code Identifiers

- **Classes**: `PascalCase` (e.g., `DataProcessor`, `UserRepository`).
- **Functions & Methods**: `lower_snake_case` (e.g., `fetch_user_data()`, `calculate_total()`).
- **Variables**: `lower_snake_case` (e.g., `user_list`, `max_retries`).
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `DEFAULT_TIMEOUT`, `API_BASE_URL`). Defined at the module level.
- **Private Identifiers**: Use a single leading underscore `_lower_snake_case` for internal variables and methods that should not be accessed from outside the class or module (e.g., `_format_query()`, `self._cache`).

## Coding Guidelines

### 1. Type Hinting (Mandatory)

Modern Python relies heavily on type hints for static analysis, IDE autocompletion, and catching bugs early. You **MUST** use type hints for all function arguments and return types.

```python
# AVOID: No type hints
def process_data(user_id, force):
    pass

# ALWAYS: Explicit type hints
def process_data(user_id: int, force: bool = False) -> dict[str, str]:
    pass
```

### 2. Modularity & Single Responsibility

- **Controllers/Routes (API)**: Should ONLY handle HTTP requests, validate input, call the service layer, and return HTTP responses. **NEVER** put database queries or complex business logic here.
- **Service Layer**: Should contain the core business logic.
- **Data Access Layer**: Should handle all database interactions (SQLalchemy, raw SQL, etc.). The rest of the app should not know _how_ data is stored.

### 3. File Execution Guard

If a file is meant to be executed directly as a script, it **MUST** include the `if __name__ == "__main__":` guard to prevent code from running unintentionally when imported elsewhere.

```python
def main() -> None:
    print("Application starting...")

if __name__ == "__main__":
    main()
```

### 4. Imports

Group imports logically at the top of the file:

1. Standard library imports (e.g., `import os`, `import sys`).
2. Third-party dependency imports (e.g., `import requests`, `from fastapi import FastAPI`).
3. Local application imports (e.g., `from src.app.core import config`).

## README.md Structure

Every Python project **MUST** include a `README.md` file in the repository root.

````md
# <Project Name>

One or two sentences describing what the Python application does and its purpose.

## Prerequisites

| Requirement  | Version                        |
| ------------ | ------------------------------ |
| **Python**   | 3.10+                          |
| **Database** | PostgreSQL 15+ (if applicable) |

## Environment Setup

1. Clone the repository.
2. Create and activate the virtual environment:
    ```bash
    python -m venv .venv
    source .venv/bin/activate  # On Windows: .venv\Scripts\activate
    ```
````

3. Install dependencies:
    ```bash
    pip install -r requirements.txt
    ```
4. Copy the environment template and configure your variables:
    ```bash
    cp .env.example .env
    ```

## Running the Application

Document the exact command needed to start the application:

```bash
# Example for a FastAPI app
uvicorn src.app.main:app --reload
```

# Example for a standard script

python src/app/main.py

```

## Architecture Notes

- **Entry point**: `src/app/main.py`
- **Frameworks used**: [e.g., FastAPI, SQLAlchemy, Click]
```

## .gitignore

Every Python project MUST include a `.gitignore` tailored to Python and OS artifacts.

```text
# Byte-compiled / optimized / DLL files
__pycache__/
*.py[cod]
*$py.class

# Virtual Environments
.venv/
venv/
ENV/
env/

# Environment Variables
.env

# Testing and Coverage
.pytest_cache/
.coverage
htmlcov/

# OS / IDE
.DS_Store
.vscode/
*.swp
```
