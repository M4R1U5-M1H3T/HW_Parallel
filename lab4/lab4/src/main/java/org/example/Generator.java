package org.example;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

public class Generator {
    static final int STUDENTS = 100;
    static final int MIN_GRADES = 70;
    static final int MAX_GRADES = 100;
    static final int MAX_GRADE = 10;
    static Random random = new Random();

    public static void main(String[] args) throws IOException {
        List<Integer> students = new ArrayList<>();
        for (int id = 1; id <= STUDENTS; id++) {
            students.add(id);
        }

        for (int project = 0; project < Grades.PROJECTS; project++) {
            Collections.shuffle(students, random);
            int numberOfGrades = MIN_GRADES + random.nextInt(MAX_GRADES - MIN_GRADES + 1);
            try (PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(Grades.projectFile(project))))) {
                for (int i = 0; i < numberOfGrades; i++) {
                    out.println(students.get(i) + " " + (1 + random.nextInt(MAX_GRADE)));
                }
            }
        }

        System.out.println("Generated project1.txt ... project" + Grades.PROJECTS + ".txt");
    }
}
