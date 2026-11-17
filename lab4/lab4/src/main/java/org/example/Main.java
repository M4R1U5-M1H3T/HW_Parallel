package org.example;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

public class Main {
    static void writeResults(String filePath, GradeList list) throws IOException {
        try (PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(filePath)))) {
            for (Node node = list.getHead(); node != null; node = node.next) {
                out.println(node.id + " " + node.grade / 10.0);
            }
        }
    }

    static boolean sameFiles(String firstPath, String secondPath) throws IOException {
        try (BufferedReader first = new BufferedReader(new FileReader(firstPath));
             BufferedReader second = new BufferedReader(new FileReader(secondPath))) {
            String firstLine = first.readLine();
            String secondLine = second.readLine();
            while (firstLine != null && secondLine != null) {
                if (!firstLine.equals(secondLine)) {
                    return false;
                }
                firstLine = first.readLine();
                secondLine = second.readLine();
            }
            return firstLine == null && secondLine == null;
        }
    }

    public static void main(String[] args) throws IOException, InterruptedException {
        String variant = args.length > 0 ? args[0] : "sequential";
        int numberOfThreads = args.length > 1 ? Integer.parseInt(args[1]) : 1;
        int numberOfReaders = args.length > 2 ? Integer.parseInt(args[2]) : 1;
        if (!variant.equals("sequential") && !variant.equals("parallel")) {
            System.out.println("Unknown variant: " + variant);
            return;
        }
        if (variant.equals("parallel") && (numberOfReaders < 1 || numberOfReaders >= numberOfThreads)) {
            System.out.println("The number of readers must be between 1 and p - 1");
            return;
        }

        for (int project = 0; project < Grades.PROJECTS; project++) {
            if (!new File(Grades.projectFile(project)).exists()) {
                System.out.println(Grades.projectFile(project) + " not found, run Generator first");
                return;
            }
        }

        long startTime = System.nanoTime();
        GradeList list = new GradeList();
        if (variant.equals("sequential")) {
            Grades.sequential(list);
        } else {
            Grades.parallel(list, numberOfThreads, numberOfReaders);
        }
        writeResults("Results.txt", list);
        long endTime = System.nanoTime();

        if (variant.equals("sequential")) {
            writeResults("sequential.txt", list);
            System.out.println("Reference");
        } else if (!new File("sequential.txt").exists()) {
            System.out.println("NotChecked");
        } else {
            System.out.println(sameFiles("Results.txt", "sequential.txt") ? "Correct" : "Wrong");
        }
        System.out.println((endTime - startTime) / 1E6);
    }
}
