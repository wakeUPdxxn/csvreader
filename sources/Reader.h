#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include "Exception.h"

class Reader
{
public:
	Reader()=default; //Определения деструктора не будет,т.к память динамически нигде не выделялась)
	void readFile();
	void openFile(const std::string& path);
	void closeFile();
private:
	enum operations{
		add = '+',
		subtract = '-',
		multiply = '*',
		divide = '/'
	};
	struct WhereToDo {   //Позиция ячейки, в которой нужно выполнить арифметическую операцию
		int row;
		int column;
	};

	std::mutex mt;           //Мьютекc для блокировки одновременного изменения ячеек таблицы 
	std::shared_mutex smt;  //Мьютекс для создания возможности параллельного чтения ячеек из таблицы 
	std::ifstream file;
	std::map<int, std::vector<std::string>>table; //таблица, где ключ-номер строки, а значение-вектор строки, состоящий из ячеек.
	std::vector<std::thread>threadVec; //Вектор потоков,которые будут заниматься обработкой ячеек с выражениями

	void cellAnalyzer(const WhereToDo& cellPos);         //функция анализа ячейки с арифметической операцией.
	std::string::iterator findOperation(std::string& currentCell);   //метод, проверящий наличие арифметической операции и возвращающий ее позицию в ячейке
	void checkArgument(const std::string& ARG);               //если возникнут ошибки с аргументом,выбросит исключение,иначе вызовет "setArgumentData"
	void setArgumentData(std::string& ARG, const std::string argData); //Заносит значение из ячеки в аргумент, на которую тот указывает.
	int makeResult(const char currentOperation,const int ARG1,const int ARG2);  //выполнение арифметической операции
	void showTabble();
	void checkRowNumber(const std::string& rowNumber);   //метод для проверки корректности номера строки
	void argumentExceptionHandler(const std::string &ARG, const int row, const int column); //вызывает функцию проверки аргумента и отлавливает исключения
};

