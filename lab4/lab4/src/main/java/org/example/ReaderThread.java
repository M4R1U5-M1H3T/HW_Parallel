package org.example;

import java.io.IOException;

public class ReaderThread extends Thread {
    private final GradeQueue queue;
    private final int firstProject, lastProject;

    ReaderThread(GradeQueue queue, int firstProject, int lastProject) {
        this.queue = queue;
        this.firstProject = firstProject;
        this.lastProject = lastProject;
    }

    @Override
    public void run() {
        try {
            Grades.readProjects(queue, firstProject, lastProject);
        } catch (IOException | InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}
