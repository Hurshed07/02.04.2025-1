#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

class ITaskRepository 
{
public:
    virtual void addTask(const string& task) = 0;
    virtual vector<string> getTasks() const = 0;
    virtual void markTaskCompleted(int index) = 0;
    virtual ~ITaskRepository() = default;
};

class InMemoryTaskRepository : public ITaskRepository 
{
private:
    struct Task 
    {
        string description;
        bool completed;
    };
    vector<Task> tasks;

public:
    void addTask(const string& task) override {
        tasks.push_back({ task, false });
    }

    vector<string> getTasks() const override 
    {
        vector<string> formattedTasks;
        for (const auto& task : tasks) {
            formattedTasks.push_back(task.completed ? "[X] " + task.description : "[ ] " + task.description);
        }
        return formattedTasks;
    }

    void markTaskCompleted(int index) override 
    {
        if (index >= 0 && index < tasks.size()) {
            tasks[index].completed = true;
        }
    }
};

class TaskManagerUI 
{
private:
    shared_ptr<ITaskRepository> repository;

public:
    explicit TaskManagerUI(shared_ptr<ITaskRepository> repo) : repository(move(repo)) {}

    void run() 
    {
        while (true) {
            cout << "\nTask Manager\n";
            cout << "1. View tasks\n";
            cout << "2. Add task\n";
            cout << "3. Mark task as completed\n";
            cout << "4. Exit\n";
            cout << "Choose an option: ";
            int choice;
            cin >> choice;
            cin.ignore(); 

            switch (choice) {
            case 1:
                viewTasks();
                break;
            case 2:
                addTask();
                break;
            case 3:
                markTaskCompleted();
                break;
            case 4:
                cout << "Exiting...\n";
                return;
            default:
                cout << "Invalid choice. Please try again.\n";
            }
        }
    }

private:
    void viewTasks() 
    {
        auto tasks = repository->getTasks();
        if (tasks.empty()) {
            cout << "No tasks available.\n";
        }
        else {
            cout << "\nTasks:\n";
            for (size_t i = 0; i < tasks.size(); ++i) {
                cout << i + 1 << ". " << tasks[i] << "\n";
            }
        }
    }

    void addTask() 
    {
        cout << "Enter a new task: ";
        string task;
        getline(cin, task);
        repository->addTask(task);
        cout << "Task added successfully.\n";
    }

    void markTaskCompleted() 
    {
        auto tasks = repository->getTasks();
        if (tasks.empty()) {
            cout << "No tasks available to mark as completed.\n";
            return;
        }

        cout << "\nTasks:\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
            cout << i + 1 << ". " << tasks[i] << "\n";
        }

        cout << "Enter the task number to mark as completed: ";
        int index;
        cin >> index;
        repository->markTaskCompleted(index - 1);
        cout << "Task marked as completed.\n";
    }
};

class FileTaskRepository : public ITaskRepository 
{
private:
    string filePath;

public:
    explicit FileTaskRepository(const string& path) : filePath(path) {}

    void addTask(const string& task) override 
    {
        ofstream file(filePath, ios::app);
        if (file.is_open()) {
            file << task << ",false\n";
            file.close();
        }
    }

    vector<string> getTasks() const override
    {
        vector<string> tasks;
        ifstream file(filePath);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                size_t commaPos = line.find(',');
                string description = line.substr(0, commaPos);
                bool completed = line.substr(commaPos + 1) == "true";
                tasks.push_back(completed ? "[X] " + description : "[ ] " + description);
            }
            file.close();
        }
        return tasks;
    }

    void markTaskCompleted(int index) override 
    {
        vector<string> lines;
        ifstream file(filePath);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        }

        if (index >= 0 && index < lines.size()) {
            size_t commaPos = lines[index].find(',');
            lines[index] = lines[index].substr(0, commaPos + 1) + "true";
        }

        ofstream outFile(filePath);
        if (outFile.is_open()) {
            for (const auto& line : lines) {
                outFile << line << "\n";
            }
            outFile.close();
        }
    }
};

int main() 
{
    auto repository = make_shared<InMemoryTaskRepository>();
    TaskManagerUI ui(repository);
    ui.run();
}
//Объяснение выбора принципов
//Single Responsibility Principle (SRP):
//Класс ITaskRepository отвечает только за хранение задач.
//Класс TaskManagerUI отвечает только за взаимодействие с пользователем.
//Это упрощает тестирование и поддержку кода.
//Open/Closed Principle (OCP):
//Мы можем добавить новую реализацию репозитория (например, FileTaskRepository) без изменения существующего кода.
//Приложение остается открытым для расширения, но закрытым для модификации.
//Dependency Inversion Principle (DIP):
//Интерфейс ITaskRepository отделяет высокоуровневый код (TaskManagerUI) от низкоуровневых деталей реализации (InMemoryTaskRepository, FileTaskRepository).
//Это позволяет легко менять способ хранения задач, например, с памяти на файл или базу данных.
