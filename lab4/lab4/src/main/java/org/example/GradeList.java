package org.example;

public class GradeList implements GradeStore {
    private Node head;

    @Override
    public synchronized void add(int id, int grade) {
        if (head == null || id < head.id) {
            head = new Node(id, grade, head);
            return;
        }
        Node current = head;
        while (current.next != null && current.next.id <= id) {
            current = current.next;
        }
        if (current.id == id) {
            current.grade += grade;
        } else {
            current.next = new Node(id, grade, current.next);
        }
    }

    public synchronized Node getHead() {
        return head;
    }
}
