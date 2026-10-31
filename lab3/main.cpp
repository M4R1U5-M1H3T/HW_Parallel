#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <mpi.h>
using namespace std;
using namespace std::chrono;

const int HEADER_WIDTH = 10;
const int TAG_SIZE = 0;
const int TAG_DIGITS = 1;
const int TAG_CARRY = 2;
const int TAG_RESULT = 3;

int processId, numberOfProcesses;
int maxLength;
bool synchronousSend = false;

struct NumberFile {
    ifstream in;
    int remainingDigits = 0;

    int open(const string& filePath) {
        in.open(filePath);
        in >> remainingDigits;
        return remainingDigits;
    }

    void readDigits(unsigned char* digits, int count) {
        for (int i = 0; i < count; i++) {
            char digit = '0';
            if (remainingDigits > 0) {
                in >> digit;
                remainingDigits--;
            }
            digits[i] = digit - '0';
        }
    }
};

NumberFile firstNumberFile, secondNumberFile;

void openNumbers() {
    int firstLength = firstNumberFile.open("Numar1.txt");
    int secondLength = secondNumberFile.open("Numar2.txt");
    maxLength = max(firstLength, secondLength);
}

void writeHeader(ofstream& out) {
    out << string(HEADER_WIDTH, ' ') << '\n';
}

void writeDigits(ofstream& out, const unsigned char* digits, int count) {
    string line(count, '0');
    for (int i = 0; i < count; i++) {
        line[i] = char('0' + digits[i]);
    }
    out << line;
}

void finishResult(ofstream& out, int finalCarry) {
    int resultLength = maxLength;
    if (finalCarry == 1) {
        out << '1';
        resultLength++;
    }
    out << '\n';
    out.seekp(0);
    out << left << setw(HEADER_WIDTH) << resultLength;
}

bool sameFiles(const string& firstPath, const string& secondPath) {
    ifstream first(firstPath, ios::binary);
    ifstream second(secondPath, ios::binary);
    string firstContent((istreambuf_iterator<char>(first)), istreambuf_iterator<char>());
    string secondContent((istreambuf_iterator<char>(second)), istreambuf_iterator<char>());
    return firstContent == secondContent;
}

int addDigits(const unsigned char* first, const unsigned char* second, unsigned char* result, int count, int carry) {
    for (int i = 0; i < count; i++) {
        int sum = first[i] + second[i] + carry;
        result[i] = sum % 10;
        carry = sum / 10;
    }
    return carry;
}

int startOf(int workerId, int totalWork, int numberOfWorkers) {
    return workerId * (totalWork / numberOfWorkers) + min(workerId, totalWork % numberOfWorkers);
}

int segmentSizeOf(int currentProcess) {
    int numberOfWorkers = numberOfProcesses - 1;
    return startOf(currentProcess, maxLength, numberOfWorkers) - startOf(currentProcess - 1, maxLength, numberOfWorkers);
}

void sendData(const void* data, int count, MPI_Datatype type, int destination, int tag) {
    if (synchronousSend) {
        MPI_Ssend(data, count, type, destination, tag, MPI_COMM_WORLD);
    } else {
        MPI_Send(data, count, type, destination, tag, MPI_COMM_WORLD);
    }
}

void receiveData(void* data, int count, MPI_Datatype type, int source, int tag) {
    MPI_Recv(data, count, type, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void sequential() {
    openNumbers();
    vector<unsigned char> firstNumber(maxLength), secondNumber(maxLength), result(maxLength);
    firstNumberFile.readDigits(firstNumber.data(), maxLength);
    secondNumberFile.readDigits(secondNumber.data(), maxLength);
    int finalCarry = addDigits(firstNumber.data(), secondNumber.data(), result.data(), maxLength, 0);

    ofstream out("Numar3.txt", ios::binary);
    writeHeader(out);
    writeDigits(out, result.data(), maxLength);
    finishResult(out, finalCarry);
}

void standardReader() {
    openNumbers();
    vector<int> segmentSizes(numberOfProcesses);
    vector<unsigned char> segment;
    for (int currentProcess = 1; currentProcess < numberOfProcesses; currentProcess++) {
        int segmentSize = segmentSizeOf(currentProcess);
        segmentSizes[currentProcess] = segmentSize;
        segment.resize(2 * segmentSize);
        firstNumberFile.readDigits(segment.data(), segmentSize);
        secondNumberFile.readDigits(segment.data() + segmentSize, segmentSize);
        sendData(&segmentSize, 1, MPI_INT, currentProcess, TAG_SIZE);
        sendData(segment.data(), 2 * segmentSize, MPI_UNSIGNED_CHAR, currentProcess, TAG_DIGITS);
    }

    ofstream out("Numar3.txt", ios::binary);
    writeHeader(out);
    vector<unsigned char> resultSegment;
    for (int currentProcess = 1; currentProcess < numberOfProcesses; currentProcess++) {
        resultSegment.resize(segmentSizes[currentProcess]);
        receiveData(resultSegment.data(), segmentSizes[currentProcess], MPI_UNSIGNED_CHAR, currentProcess, TAG_RESULT);
        writeDigits(out, resultSegment.data(), segmentSizes[currentProcess]);
    }
    int finalCarry;
    receiveData(&finalCarry, 1, MPI_INT, numberOfProcesses - 1, TAG_CARRY);
    finishResult(out, finalCarry);
}

vector<unsigned char> receiveSegment(int& segmentSize) {
    receiveData(&segmentSize, 1, MPI_INT, 0, TAG_SIZE);
    vector<unsigned char> segment(2 * segmentSize);
    receiveData(segment.data(), 2 * segmentSize, MPI_UNSIGNED_CHAR, 0, TAG_DIGITS);
    return segment;
}

bool isLastProcess() {
    return processId == numberOfProcesses - 1;
}

void sendCarryToNext(int carry) {
    if (!isLastProcess()) {
        sendData(&carry, 1, MPI_INT, processId + 1, TAG_CARRY);
    }
}

void sendResult(const vector<unsigned char>& result, int carry) {
    sendData(result.data(), (int) result.size(), MPI_UNSIGNED_CHAR, 0, TAG_RESULT);
    if (isLastProcess()) {
        sendData(&carry, 1, MPI_INT, 0, TAG_CARRY);
    }
}

void standardWorker() {
    int segmentSize;
    vector<unsigned char> segment = receiveSegment(segmentSize);
    int carry = 0;
    if (processId > 1) {
        receiveData(&carry, 1, MPI_INT, processId - 1, TAG_CARRY);
    }
    vector<unsigned char> result(segmentSize);
    carry = addDigits(segment.data(), segment.data() + segmentSize, result.data(), segmentSize, carry);
    sendCarryToNext(carry);
    sendResult(result, carry);
}

bool allNines(const vector<unsigned char>& digits) {
    for (unsigned char digit : digits) {
        if (digit != 9) {
            return false;
        }
    }
    return true;
}

void addOne(vector<unsigned char>& digits) {
    size_t i = 0;
    while (i < digits.size() && digits[i] == 9) {
        digits[i] = 0;
        i++;
    }
    if (i < digits.size()) {
        digits[i]++;
    }
}

void optimizedWorker() {
    int segmentSize;
    vector<unsigned char> segment = receiveSegment(segmentSize);
    vector<unsigned char> result(segmentSize);
    int carry = addDigits(segment.data(), segment.data() + segmentSize, result.data(), segmentSize, 0);
    bool carryDependsOnPrevious = allNines(result);
    if (!carryDependsOnPrevious) {
        sendCarryToNext(carry);
    }

    int previousCarry = 0;
    if (processId > 1) {
        receiveData(&previousCarry, 1, MPI_INT, processId - 1, TAG_CARRY);
    }
    if (previousCarry == 1) {
        addOne(result);
    }
    if (carryDependsOnPrevious) {
        carry = previousCarry;
        sendCarryToNext(carry);
    }
    sendResult(result, carry);
}

void scatterGather() {
    int segmentSize, paddedLength = 0;
    vector<unsigned char> firstNumber, secondNumber, result;
    if (processId == 0) {
        openNumbers();
        paddedLength = (maxLength + numberOfProcesses - 1) / numberOfProcesses * numberOfProcesses;
        segmentSize = paddedLength / numberOfProcesses;
        firstNumber.resize(paddedLength);
        secondNumber.resize(paddedLength);
        result.resize(paddedLength);
        firstNumberFile.readDigits(firstNumber.data(), paddedLength);
        secondNumberFile.readDigits(secondNumber.data(), paddedLength);
    }
    MPI_Bcast(&segmentSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<unsigned char> firstSegment(segmentSize), secondSegment(segmentSize), resultSegment(segmentSize);
    MPI_Scatter(firstNumber.data(), segmentSize, MPI_UNSIGNED_CHAR,
            firstSegment.data(), segmentSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Scatter(secondNumber.data(), segmentSize, MPI_UNSIGNED_CHAR,
            secondSegment.data(), segmentSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    int carry = 0;
    if (processId > 0) {
        receiveData(&carry, 1, MPI_INT, processId - 1, TAG_CARRY);
    }
    carry = addDigits(firstSegment.data(), secondSegment.data(), resultSegment.data(), segmentSize, carry);
    sendCarryToNext(carry);

    MPI_Gather(resultSegment.data(), segmentSize, MPI_UNSIGNED_CHAR,
            result.data(), segmentSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&carry, 1, MPI_INT, numberOfProcesses - 1, MPI_COMM_WORLD);

    if (processId == 0) {
        int finalCarry = paddedLength > maxLength ? result[maxLength] : carry;
        ofstream out("Numar3.txt", ios::binary);
        writeHeader(out);
        writeDigits(out, result.data(), maxLength);
        finishResult(out, finalCarry);
    }
}

void asyncReader() {
    openNumbers();
    vector<int> segmentSizes(numberOfProcesses);
    vector<vector<unsigned char>> segments(numberOfProcesses);
    vector<MPI_Request> sendRequests(2 * numberOfProcesses, MPI_REQUEST_NULL);
    for (int currentProcess = 1; currentProcess < numberOfProcesses; currentProcess++) {
        int segmentSize = segmentSizeOf(currentProcess);
        segmentSizes[currentProcess] = segmentSize;
        segments[currentProcess].resize(2 * segmentSize);
        firstNumberFile.readDigits(segments[currentProcess].data(), segmentSize);
        secondNumberFile.readDigits(segments[currentProcess].data() + segmentSize, segmentSize);
        MPI_Isend(&segmentSizes[currentProcess], 1, MPI_INT, currentProcess, TAG_SIZE,
                MPI_COMM_WORLD, &sendRequests[2 * currentProcess]);
        MPI_Isend(segments[currentProcess].data(), 2 * segmentSize, MPI_UNSIGNED_CHAR, currentProcess, TAG_DIGITS,
                MPI_COMM_WORLD, &sendRequests[2 * currentProcess + 1]);
    }

    vector<vector<unsigned char>> resultSegments(numberOfProcesses);
    vector<MPI_Request> resultRequests(numberOfProcesses, MPI_REQUEST_NULL);
    for (int currentProcess = 1; currentProcess < numberOfProcesses; currentProcess++) {
        resultSegments[currentProcess].resize(segmentSizes[currentProcess]);
        MPI_Irecv(resultSegments[currentProcess].data(), segmentSizes[currentProcess], MPI_UNSIGNED_CHAR,
                currentProcess, TAG_RESULT, MPI_COMM_WORLD, &resultRequests[currentProcess]);
    }
    int finalCarry;
    MPI_Request carryRequest;
    MPI_Irecv(&finalCarry, 1, MPI_INT, numberOfProcesses - 1, TAG_CARRY, MPI_COMM_WORLD, &carryRequest);

    ofstream out("Numar3.txt", ios::binary);
    writeHeader(out);
    for (int currentProcess = 1; currentProcess < numberOfProcesses; currentProcess++) {
        MPI_Wait(&resultRequests[currentProcess], MPI_STATUS_IGNORE);
        writeDigits(out, resultSegments[currentProcess].data(), segmentSizes[currentProcess]);
    }
    MPI_Wait(&carryRequest, MPI_STATUS_IGNORE);
    finishResult(out, finalCarry);
    MPI_Waitall((int) sendRequests.size(), sendRequests.data(), MPI_STATUSES_IGNORE);
}

void asyncWorker() {
    int segmentSize;
    int carry = 0;
    MPI_Request sizeRequest, digitsRequest;
    MPI_Request carryRequest = MPI_REQUEST_NULL;
    MPI_Irecv(&segmentSize, 1, MPI_INT, 0, TAG_SIZE, MPI_COMM_WORLD, &sizeRequest);
    if (processId > 1) {
        MPI_Irecv(&carry, 1, MPI_INT, processId - 1, TAG_CARRY, MPI_COMM_WORLD, &carryRequest);
    }
    MPI_Wait(&sizeRequest, MPI_STATUS_IGNORE);
    vector<unsigned char> segment(2 * segmentSize);
    MPI_Irecv(segment.data(), 2 * segmentSize, MPI_UNSIGNED_CHAR, 0, TAG_DIGITS, MPI_COMM_WORLD, &digitsRequest);
    MPI_Wait(&digitsRequest, MPI_STATUS_IGNORE);
    MPI_Wait(&carryRequest, MPI_STATUS_IGNORE);

    vector<unsigned char> result(segmentSize);
    carry = addDigits(segment.data(), segment.data() + segmentSize, result.data(), segmentSize, carry);

    MPI_Request sendRequests[2];
    int nextProcess = isLastProcess() ? 0 : processId + 1;
    MPI_Isend(&carry, 1, MPI_INT, nextProcess, TAG_CARRY, MPI_COMM_WORLD, &sendRequests[0]);
    MPI_Isend(result.data(), segmentSize, MPI_UNSIGNED_CHAR, 0, TAG_RESULT, MPI_COMM_WORLD, &sendRequests[1]);
    MPI_Waitall(2, sendRequests, MPI_STATUSES_IGNORE);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    string variant = argc > 1 ? argv[1] : "sequential";
    synchronousSend = argc > 2 && string(argv[2]) == "ssend";
    bool needsWorkers = variant == "standard" || variant == "optimized" || variant == "async";
    if (!needsWorkers && variant != "sequential" && variant != "scatter") {
        if (processId == 0) {
            cout << "Unknown variant: " << variant << '\n';
        }
        MPI_Finalize();
        return 1;
    }
    if (needsWorkers && numberOfProcesses < 2) {
        cout << "Variant " << variant << " needs at least 2 processes\n";
        MPI_Finalize();
        return 1;
    }
    if (processId == 0 && (!ifstream("Numar1.txt") || !ifstream("Numar2.txt"))) {
        cout << "Numar1.txt or Numar2.txt not found, run generator first\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    auto startTime = high_resolution_clock::now();
    if (variant == "sequential") {
        if (processId == 0) {
            sequential();
        }
    } else if (variant == "scatter") {
        scatterGather();
    } else if (processId == 0) {
        variant == "async" ? asyncReader() : standardReader();
    } else if (variant == "standard") {
        standardWorker();
    } else if (variant == "optimized") {
        optimizedWorker();
    } else {
        asyncWorker();
    }
    auto endTime = high_resolution_clock::now();

    if (processId == 0) {
        if (variant == "sequential") {
            filesystem::copy_file("Numar3.txt", "sequential.txt", filesystem::copy_options::overwrite_existing);
            cout << "Reference\n";
        } else if (!ifstream("sequential.txt")) {
            cout << "NotChecked\n";
        } else {
            cout << (sameFiles("Numar3.txt", "sequential.txt") ? "Correct" : "Wrong") << '\n';
        }
        duration<double, milli> executionTime = endTime - startTime;
        cout << executionTime.count() << '\n';
    }

    MPI_Finalize();
    return 0;
}
