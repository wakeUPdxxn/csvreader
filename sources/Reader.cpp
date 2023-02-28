#include "Reader.h"                             

void Reader::readFile() {
    std::vector<std::string> row;               //вектор ячеек строки из файла
    std::string currentRow,currentCell;         //текущая строка и текущая ячейка     

    while (getline(file, currentRow)) {  //считываем строку из файла
        int columnCounter = 0, rowNumber=0;       //счетчик колонок и номер текущей строки
        static int stepCounter = 0;
        std::stringstream ss(currentRow);

        while (getline(ss, currentCell, ',')) { //Разбираем строку на ячейки и заносим их в вектор текущей строки
            row.push_back(currentCell);
            if (stepCounter != 0 && columnCounter ==0) { //Условие которое срабатывает только на первом элементе каждой строки
                try {
                    checkRowNumber(row[0]);      //Проверяем номер строки
                    rowNumber = stoi(row[0]);   //Если номер валиден, копируем его в номер текущей строки как число
                }
                catch (IncorrectRowNumberException x) {
                    std::cerr << "Incorrect row number in row: " << stepCounter << "\n";
                    exit(EXIT_FAILURE);
                }
                catch (NonUniqueRowsException x) {
                    std::cerr << "Non-unique row numbers in table" << "\n";
                    exit(EXIT_FAILURE);
                }
            }
            if (size_t isOperatorExist = currentCell.find('=') != std::string::npos) {  //Проверяем,нужно ли что-то считать в ячейке
                WhereToDo cellPos = { rowNumber,columnCounter };                       //Если да,запоминаем позицию ячейки в виде структуры
                threadVec.push_back(std::thread(&Reader::cellAnalyzer,this,cellPos)); //Создаем потоки, которые будут выполнять анализ ячеек с операцией и заносим их в вектор.
            }
            currentCell.clear();
            columnCounter++;
        }
        table[rowNumber] = row;  //заносим в map строку с ключом,который предствляет себя номер строки из её первого столбца
        stepCounter++;
        row.clear();
    }  
    for (auto& th : threadVec) {
        th.join();         //Запускаем потоки анализа ячеек с операцией 
    }
    showTabble(); //Т.к освной поток блокируется при вызове th.join,то функция вывода таблицы будет вызвана, как только потоки завершат свое выполнение
}

void Reader::openFile(const std::string& path) {
    file.open(path.c_str(), std::ios::out);
    if (!file.is_open()) {
        throw(FileOpenException());
    }
}

void Reader::closeFile() { 
    file.close();
}

void Reader::showTabble() {
    for (const auto& row : table) {
        for (auto it = row.second.begin(); it != row.second.end();++it) {
            if (it != row.second.end() - 1) { 
                std::cout << *it + ','; 
            }
            else { std::cout << *it; }
        }
        std::cout << "\n";
    }
}

void Reader::checkRowNumber(const std::string &rowNumber) {
    static std::string prevRowNum;
    if (prevRowNum == rowNumber) { 
        throw(NonUniqueRowsException());
    }
    for (const auto& digit : rowNumber) {
        if (!isdigit(digit)) {
            throw(IncorrectRowNumberException());
        }
    }
    prevRowNum = rowNumber;
}

void Reader::cellAnalyzer(const WhereToDo &cellPos) {
    int column = cellPos.column;
    int row = cellPos.row;

    smt.lock_shared();
    std::string currentCell = table[row].at(column);  //Заносим из таблицы в ячейку,где нужно что-то вычислить, в строку
    smt.unlock_shared();

    std::string ARG1, ARG2;
    char currentOperation = '0';
    std::string::iterator operationPos;

    try { 
        operationPos = findOperation(currentCell); 
        currentOperation = *operationPos;          //Если функция поиска и проверки операции не выборосила исключение,то заносим символ текущей операции
    }
    catch(WrongOperator x) {
        mt.lock();
        std::cerr << "Unidentified or doesn't existing operation" << " in row: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }     
    if (operationPos == currentCell.begin() + 1) {        //Если перед оператором ничего нет, то выводим ошибку об отсутствии 1го аргумента
        mt.lock();
        std::cerr << "No ARG1 in: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
    else if (operationPos == currentCell.end() - 1) {     //Аналогично,если после оператора пусто
        mt.lock();
        std::cerr << "No ARG2 in: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }

    ARG1 = currentCell.substr(std::distance(currentCell.begin(), currentCell.begin()+1), std::distance(currentCell.begin(),operationPos-1)); //Формируем первый аргумент из ячейки с операцией
    argumentExceptionHandler(ARG1, row, column); //Вызываем функцию для захвата исключений при проверке аргумента, если таковых не было, последняя вызовет сеттер значения для аргумента 

    ARG2 = currentCell.substr(std::distance(currentCell.begin(), operationPos + 1), std::distance(operationPos + 1, currentCell.end()));
    argumentExceptionHandler(ARG2, row, column); //Аналогично для второго аргумента

    try {
        std::lock_guard<std::mutex>lock(mt); //Захват мьютекса для единоличного использования таблицы с целью изменения текущей ячейки
        table[row].at(column) = std::to_string(makeResult(currentOperation, stoi(ARG1), stoi(ARG2))); //Заменяем значение в ячейке результатом операции
    }
    catch (DevisionByZeroException x) {
        mt.unlock();
        std::cerr << "Division by zero is not allowed" << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
    catch(std::exception &e){}
}

std::string::iterator Reader::findOperation(std::string &currentCell){
    auto operationPos = std::find_if(currentCell.begin(), currentCell.end(), [](const char& elem) { //Проверяем, есть ли операция из ячейки среди доступных
        return (elem == '+' || elem == '-' || elem == '*' || elem == '/');
        });
    if (operationPos != currentCell.end()) {      //Если нет, выбрасываем исключение, иначе вернем итератор на эту операцию
        return operationPos;
    }
    else { throw(WrongOperator()); }
}

void Reader::checkArgument(const std::string &ARG) {
    std::string argRow;             //Номер строки из аргумента
    std::string argColumnName;     //Имя колонки аргумента
    std::string argData;          //Значение ячейки на которую ссылается аргумент

    if (isdigit(ARG[0])) {                        //Если аргумент-число,проверим,нет ли в нем чего-нибудь лишнего
        if (!std::all_of(ARG.begin(), ARG.end(), [](const char& elem) { return isdigit(elem); })) { 
            throw(WrongArgumentSpec());                                                             
        }      
    }
    else if(isalpha(ARG[0])) {                  //Если аргумент начинается с имени колонки,проверяем, указан ли номер строки
        auto rowPos = std::find_if(ARG.begin(), ARG.end(), [](const char& elem) {
            return isdigit(elem);
            });
        if (rowPos != ARG.end()) {             //Если аргумент содержит имя колонки и номер строки,проверяем их
            argRow.assign(rowPos, ARG.end());
            if (!std::all_of(argRow.begin(), argRow.end(), [](const char& elem) { return isdigit(elem); })) { //Проверим корректность номера строки из аргумента
                throw(WrongArgumentSpec());
            }
            argColumnName.assign(ARG.begin(), rowPos); 
            auto isArgExistInHeader = std::find(table[0].begin(), table[0].end(), argColumnName); //Ищем колонку с таким именем в заголовке

            if (isArgExistInHeader != table[0].end()) {         //Если колонка существует в заголовке, ищем строку
                auto isArgRowExist = table.find(stoi(argRow));

                if (isArgRowExist != table.end()) {            //Если строка и колонка существуют,написание аргумента считается корректным, можем идти дальше
                    smt.lock_shared();
                    argData = table[stoi(argRow)].at(std::distance(table[0].begin(), isArgExistInHeader)); //Вытаскиваем значение из ячейки, на которую ссылается аргумент
                    smt.unlock_shared();
                    if (argData.find(ARG) != std::string::npos) { //Если аргумент ссылается на текущую ячейку, выбрасываем исключение.
                        throw(SelfLinkException());
                    }
                    if (argData[0] == '=') { 
                        throw(UndefiendReferencedCellData());    //Если значение ссылаемой ячейки ещё не посчитано, выбросим соответствующее исключение 
                    }  
                    else if (!isdigit(argData[0])) {  //Иначе, если значение ячейки, на которую ссылкается аргумент,невалидно
                        throw(WrongReferendCellData());         
                    }    
                    else { setArgumentData(const_cast<std::string&>(ARG), argData); } //Если все хорошо,заносим в аргумент значение ячейки, на которую тот указывает.
                }
                else { throw(WrongRowInArg()); }
            }
            else { throw(WrongColumnInArg()); }
        }
        else { throw(UnaddressedArgrument()); }
    }
}

void Reader::setArgumentData(std::string& ARG, const std::string argData)
{
    ARG = argData;
}

int Reader::makeResult(const char currentOperation, const int ARG1, const int ARG2) {
    switch (currentOperation)             //Выполняем соответствуеющую операцию над аргументами
    {
    case operations::add:
        return ARG1 + ARG2;
        break;
    case operations::divide:
        if (ARG2 == 0) {
            throw(DevisionByZeroException());
        }
        return ARG1 / ARG2;
        break;
    case operations::multiply:
        return ARG1 * ARG2;
        break;
    case operations::subtract:
        return ARG1 - ARG2;
        break;
    default:
        break;
    }
}

void Reader::argumentExceptionHandler(const std::string &ARG,const int row,const int column) {  //В этом методе ловим все исключения для аргумента
    try { checkArgument(ARG); }  //Проверяем валидность аргумента
    catch (WrongArgumentSpec x) {
        mt.lock();
        std::cerr << "Incorrect argument specification" << " in row: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
    catch (UndefiendReferencedCellData x) {   //Если значение ячейки, на которую ссылкается аргумент неопределено, операция в текущей ячейке не может быть выполнена
        static int attempCounter = 1;
        if (attempCounter >= 100) {
            mt.lock();
            std::cerr << "Looks like there is an unresolved dependency in table cells. Emergency exit!" << "\n";
            mt.unlock();
            exit(EXIT_FAILURE);
        }
        attempCounter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); //"усыпляем" текущий поток,с расчетом на то, что другой поток посчитает значение нужной ячейки
        cellAnalyzer(WhereToDo{row,column});  //Отправляем ячейку для повторного анализа
    }
    catch (WrongReferendCellData x) {
        mt.lock();
        std::cerr << "Invalid value in cell referenced by the argument" << " in row: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
    catch (WrongRowInArg x) {
        mt.lock();
        std::cerr << "There is no such ARG row number in cell" << " in row: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
    catch (WrongColumnInArg x) {
        mt.lock();
        std::cerr << "There is no such ARG in the table header.Cause" << " in row: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
    catch (UnaddressedArgrument x) {
        mt.lock();
        std::cerr << "No address specified for ARG" << " in row: " << row << " column: " << column << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
    catch (SelfLinkException x) {
        mt.lock();
        std::cerr << "Self linking argument" << " in row: " << row+1 << " column: " << column+1 << "\n";
        mt.unlock();
        exit(EXIT_FAILURE);
    }
}
