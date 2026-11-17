package org.example;

public class WorkerThread extends Thread {
    private final GradeQueue queue;
    private final GradeList list;

    WorkerThread(GradeQueue queue, GradeList list) {
        this.queue = queue;
        this.list = list;
    }

    @Override
    public void run() {
        try {
            Node node = queue.take();
            while (node.id != Grades.END_ID) {
                list.add(node.id, node.grade);
                node = queue.take();
            }
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}
