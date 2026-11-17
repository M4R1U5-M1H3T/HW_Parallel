package org.example;

import java.util.concurrent.Semaphore;

public class GradeQueue implements GradeStore {
    private final Semaphore mutex = new Semaphore(1);
    private final Semaphore available = new Semaphore(0);
    private Node head, tail;

    @Override
    public void add(int id, int grade) throws InterruptedException {
        Node node = new Node(id, grade, null);
        mutex.acquire();
        if (tail == null) {
            head = node;
        } else {
            tail.next = node;
        }
        tail = node;
        mutex.release();
        available.release();
    }

    public Node take() throws InterruptedException {
        available.acquire();
        mutex.acquire();
        Node node = head;
        head = head.next;
        if (head == null) {
            tail = null;
        }
        mutex.release();
        return node;
    }
}
