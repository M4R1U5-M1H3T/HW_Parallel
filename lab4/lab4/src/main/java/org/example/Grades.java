package org.example;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class Grades {
    static final int PROJECTS = 10;
    static final int END_ID = -1;

    static String projectFile(int project) {
        return "project" + (project + 1) + ".txt";
    }

    static int startOf(int threadId, int totalWork, int numberOfThreads) {
        return threadId * (totalWork / numberOfThreads) + Math.min(threadId, totalWork % numberOfThreads);
    }

    static void readProjects(GradeStore store, int firstProject, int lastProject) throws IOException, InterruptedException {
        for (int project = firstProject; project < lastProject; project++) {
            try (BufferedReader in = new BufferedReader(new FileReader(projectFile(project)))) {
                String line = in.readLine();
                while (line != null) {
                    String[] pair = line.trim().split(" ");
                    store.add(Integer.parseInt(pair[0]), Integer.parseInt(pair[1]));
                    line = in.readLine();
                }
            }
        }
    }

    static void sequential(GradeList list) throws IOException, InterruptedException {
        readProjects(list, 0, PROJECTS);
    }

    static void parallel(GradeList list, int numberOfThreads, int numberOfReaders) throws IOException, InterruptedException {
        int numberOfWorkers = numberOfThreads - numberOfReaders;
        GradeQueue queue = new GradeQueue();

        Thread[] readers = new Thread[numberOfReaders - 1];
        for (int readerId = 1; readerId < numberOfReaders; readerId++) {
            readers[readerId - 1] = new ReaderThread(queue, startOf(readerId, PROJECTS, numberOfReaders),
                    startOf(readerId + 1, PROJECTS, numberOfReaders));
        }
        Thread[] workers = new Thread[numberOfWorkers];
        for (int workerId = 0; workerId < numberOfWorkers; workerId++) {
            workers[workerId] = new WorkerThread(queue, list);
        }
        start(readers);
        start(workers);

        readProjects(queue, startOf(0, PROJECTS, numberOfReaders), startOf(1, PROJECTS, numberOfReaders));
        join(readers);
        for (int workerId = 0; workerId < numberOfWorkers; workerId++) {
            queue.add(END_ID, 0);
        }
        join(workers);
    }

    static void start(Thread[] threads) {
        for (Thread thread : threads) {
            thread.start();
        }
    }

    static void join(Thread[] threads) throws InterruptedException {
        for (Thread thread : threads) {
            thread.join();
        }
    }
}
