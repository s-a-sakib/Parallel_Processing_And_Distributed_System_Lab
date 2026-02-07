//Simplified
#include <bits/stdc++.h>
#include <mpi.h>
using namespace std;

struct Contact {
    string name;
    string phone;
};

void send_string(const string &text, int receiver) {
    int len = text.size() + 1;
    MPI_Send(&len, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD);
    MPI_Send(text.c_str(), len, MPI_CHAR, receiver, 1, MPI_COMM_WORLD);
}

void receive_string(string &text, int sender) {
    int len;
    MPI_Recv(&len, 1, MPI_INT, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char buffer[len];
    MPI_Recv(buffer, len, MPI_CHAR, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    text = string(buffer);
}

vector <Contact> string_to_contacts(const string &data){
    vector <Contact> contacts;
    stringstream ss(data);
    string line;
    while (getline(ss, line)) {
        size_t pos = line.find(',');
        if (pos != string::npos) {
            Contact contact;
            contact.name = line.substr(0, pos);
            contact.phone = line.substr(pos + 1);
            contacts.push_back(contact);
        }
    }
    return contacts;
}

string check(const Contact &contact, const string &query) {
    if (contact.name.find(query) != string::npos) {
        return contact.name + ": " + contact.phone;
    }
    return "";
}

void read_file(const string &filename, vector <Contact> &contacts) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        size_t pos = line.find(',');
        if (pos != string::npos) {
            Contact contact;
            contact.name = line.substr(0, pos);
            contact.phone = line.substr(pos + 1);
            contacts.push_back(contact);
        }
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    string filename = "phonebook1.txt";
    string search_term;

    // ---- Take input from terminal (only rank 0) ----
    if (rank == 0) {
        cout << "Enter search term: ";
        cin >> search_term;
    }

    // ---- Broadcast search term to all processes ----
    char query_buf[100] = {0};
    if (rank == 0) strncpy(query_buf, search_term.c_str(), 99);
    MPI_Bcast(query_buf, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
    search_term = query_buf;

    double start, end;

    if (rank == 0) {
        vector<string> files;
        files.push_back(filename);

        vector<Contact> contacts;
        read_phonebook(files, contacts);

        int total = contacts.size();
        int chunk = (total + size - 1) / size;

        // Send chunks to workers
        for (int i = 1; i < size; i++) {
            string text = vector_to_string(contacts, i * chunk, (i + 1) * chunk);
            send_string(text, i);
        }

        start = MPI_Wtime();

        // Root searches its own chunk
        string result;
        for (int i = 0; i < min(chunk, total); i++) {
            string match = check(contacts[i], search_term);
            if (!match.empty()) result += match;
        }

        // Collect worker results
        for (int i = 1; i < size; i++) {
            string recv = receive_string(i);
            if (!recv.empty()) result += recv;
        }

        end = MPI_Wtime();

        ofstream out("output.txt");
        out << result;
        out.close();

        cout << "Results saved to output.txt\n";
        printf("Process %d took %f seconds.\n", rank, end - start);

    } else {
        // Worker side
        string recv_text = receive_string(0);
        vector<Contact> contacts = string_to_contacts(recv_text);

        start = MPI_Wtime();

        string result;
        for (auto &c : contacts) {
            string match = check(c, search_term);
            if (!match.empty()) result += match;
        }

        end = MPI_Wtime();

        send_string(result, 0);
        printf("Process %d took %f seconds.\n", rank, end - start);
    }

    MPI_Finalize();
    return 0;
}
