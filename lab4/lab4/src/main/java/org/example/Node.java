package org.example;

public class Node {
    final int id;
    int grade;
    Node next;

    Node(int id, int grade, Node next) {
        this.id = id;
        this.grade = grade;
        this.next = next;
    }
}
