#include <bits/stdc++.h>
#include <mpi.h>
using namespace std;

struct Contact {
    string name;
    string phone;
};

void send_string(const string &text, int receiver, int tag=1) {
    int len = (int)text.size() + 1;
    MPI_Send(&len, 1, MPI_INT, receiver, tag, MPI_COMM_WORLD);
    MPI_Send(text.c_str(), len, MPI_CHAR, receiver, tag, MPI_COMM_WORLD);
}

string receive_string(int sender, int tag=1) {
    int len;
    MPI_Recv(&len, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    vector<char> buffer(len);
    MPI_Recv(buffer.data(), len, MPI_CHAR, sender, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return string(buffer.data());
}

vector<Contact> string_to_contacts(const string &data) {
    vector<Contact> contacts;
    stringstream ss(data);
    string line;
    while (getline(ss, line)) {
        if (line.empty()) continue;
        size_t pos = line.find(',');
        if (pos == string::npos) continue;
        Contact c;
        c.name = line.substr(0, pos);
        c.phone = line.substr(pos + 1);
        contacts.push_back(c);
    }
    return contacts;
}

string contacts_to_string(const vector<Contact> &contacts, int start, int end) {
    string out;
    for (int i = start; i < min((int)contacts.size(), end); i++) {
        out += contacts[i].name + "," + contacts[i].phone + "\n";
    }
    return out;
}

string check(const Contact &contact, const string &query) {
    if (contact.name.find(query) != string::npos) {
        return contact.name + ": " + contact.phone + "\n";
    }
    return "";
}

void read_file(const string &filename, vector<Contact> &contacts) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open file: " << filename << "\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        size_t pos = line.find(',');
        if (pos == string::npos) continue;
        Contact c;
        c.name = line.substr(0, pos);
        c.phone = line.substr(pos + 1);
        contacts.push_back(c);
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    string filename = "phonebook1.txt";
    string search_term;

    // Input only on rank 0
    if (rank == 0) {
        cout << "Enter search term: ";
        cin >> search_term;
    }

    // Broadcast search term
    char query_buf[100] = {0};
    if (rank == 0) strncpy(query_buf, search_term.c_str(), 99);
    MPI_Bcast(query_buf, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
    search_term = query_buf;

    double start, end;

    if (rank == 0) {
        vector<Contact> contacts;
        read_file(filename, contacts);

        int total = (int)contacts.size();
        int chunk = (total + size - 1) / size;   // ceil

        // Send chunks to workers
        for (int i = 1; i < size; i++) {
            string text = contacts_to_string(contacts, i * chunk, (i + 1) * chunk);
            send_string(text, i);
        }

        start = MPI_Wtime();

        // Root searches its own chunk
        string result;
        for (int i = 0; i < min(chunk, total); i++) {
            result += check(contacts[i], search_term);
        }

        // Collect results
        for (int i = 1; i < size; i++) {
            string recv = receive_string(i);
            result += recv;
        }

        end = MPI_Wtime();

        ofstream out("output.txt");
        out << result;
        out.close();

        cout << "Results saved to output.txt\n";
        printf("Rank %d total time: %f seconds.\n", rank, end - start);

    } else {
        // Worker side
        string recv_text = receive_string(0);
        vector<Contact> contacts = string_to_contacts(recv_text);

        start = MPI_Wtime();

        string result;
        for (auto &c : contacts) {
            result += check(c, search_term);
        }

        end = MPI_Wtime();

        send_string(result, 0);
        printf("Rank %d local time: %f seconds.\n", rank, end - start);
    }

    MPI_Finalize();
    return 0;
}