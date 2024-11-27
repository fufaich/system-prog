#ifndef DATABASE_H
#define DATABASE_H

// Создание базы данных
int createdb();

// Подключение к БД
int connectdb();

// Отключение от БД
int disconnectdb();

// Добавление пары ключ-значение
int add(int key, const char* value);

// Удаление записи по ключу
int delete(int key);

// Поиск записи по ключу
const char* search(int key);

#endif // DATABASE_H
