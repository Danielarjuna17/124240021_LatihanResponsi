#include <iostream>
#include <stdio.h>
#include <cstring>
#include <iomanip>
using namespace std;

FILE *latres;

struct Video{
    char judul[40];
    int durasi;
    char status[25];
    Video *left;
    Video *right;
};

struct Aksi {
    char tipe[20];           // "tambah", "hapus", "playlist", "tonton"
    Video* data;             // Simpan pointer ke video terkait
    int posisi;              // Untuk playlist atau riwayat (jika diperlukan)
};

Aksi aksiTerakhir = {"none", nullptr, -1};


// Data yang disimpan untuk undo
Video* videoUndo = nullptr;
int posisiUndo = -1; // Untuk undo playlist/tonton


// Root BST global sementara (atau bisa juga lewat parameter jika kamu modular)
Video *root = nullptr;

// Buat node baru
Video* buatNode(const char* judul, int durasi, const char* status) {
    Video* baru = new Video;
    strcpy(baru->judul, judul);
    baru->durasi = durasi;
    strcpy(baru->status, status);
    baru->left = nullptr;
    baru->right = nullptr;
    return baru;
}

// Fungsi insert dengan void dan reference pointer
void insertVideo(Video *&root, Video *baru) {
    if (root == nullptr) {
        root = baru;
        cout << "Video \"" << baru->judul << "\" berhasil ditambahkan.\n";
        return;
    }

    Video *current = root;
    while (true) {
        int cmp = strcmp(baru->judul, current->judul);
        if (cmp == 0) {
            cout << "Judul \"" << baru->judul << "\" sudah ada. Tidak ditambahkan.\n";
            delete baru; // karena duplikat, hapus node yang dibuat
            return;
        } else if (cmp < 0) {
            if (current->left == nullptr) {
                current->left = baru;
                cout << "Video \"" << baru->judul << "\" berhasil ditambahkan di kiri.\n";
                return;
            }
            current = current->left;
        } else {
            if (current->right == nullptr) {
                current->right = baru;
                cout << "Video \"" << baru->judul << "\" berhasil ditambahkan di kanan.\n";
                return;
            }
            current = current->right;
        }
    }
}

// Simpan node BST ke file (preorder)
void simpanKeFile(Video* node, FILE* file) {
    if (node == nullptr) return;

    // Simpan hanya data, tanpa pointer left/right
    fwrite(node, sizeof(Video) - sizeof(Video*) * 2, 1, file);

    simpanKeFile(node->left, file);
    simpanKeFile(node->right, file);
}

// Fungsi untuk input dari user dan masukkan ke BST
void tambahVideo() {
    int jumlah;
    cout << "Masukkan jumlah video yang ingin ditambahkan: ";
    cin >> jumlah;
    if (jumlah <= 0) {
        cout << "Jumlah tidak valid!\n";
        return;
    }

    cin.ignore(); // flush newline sebelum loop
    for (int i = 0; i < jumlah; i++) {
        char judul[40];
        int durasi;
        char status[25];

        cout << "\nVideo ke-" << (i + 1) << ":\n";
        cout << "Judul Video : ";
        cin.getline(judul, 40);
        cout << "Durasi (menit) : ";
        cin >> durasi;
        cin.ignore(); // flush newline
        cout << "Status (Tersedia / Dalam Antrean / Sedang Diputar): ";
        cin.getline(status, 25);

        Video *baru = buatNode(judul, durasi, status);
        insertVideo(root, baru);
    }

    // Simpan ke file
    latres = fopen("IDLIX_Tube.dat", "ab");
    if (latres == nullptr) {
        cout << "Gagal membuka file!\n";
        return;
    }
    simpanKeFile(root, latres);
    fclose(latres);
    cout << "\nSemua video berhasil disimpan ke file.\n";
}

void tampilkanVideo() {
    latres = fopen("IDLIX_Tube.dat", "rb");
    if (latres == NULL) {
        cout << "Gagal membuka file video.\n";
        return;
    }

    Video daftar[100];
    int total = 0;

    while (fread(&daftar[total], sizeof(Video) - sizeof(Video*) * 2, 1, latres) == 1) {
    if (total >= 100) break;
    total++;
}
    fclose(latres);

    // Urutkan berdasarkan judul secara ascending (bubble sort)
    for (int i = 0; i < total - 1; i++) {
        for (int j = i + 1; j < total; j++) {
            if (strcmp(daftar[i].judul, daftar[j].judul) > 0) {
                Video temp = daftar[i];
                daftar[i] = daftar[j];
                daftar[j] = temp;
            }
        }
    }

    // Tampilkan daftar video
    cout << "\nDaftar Video (Urut Judul A-Z):\n";
    cout << "-------------------------------------------------------------\n";
    cout << left << setw(30) << "Judul"
         << setw(10) << "Durasi"
         << setw(20) << "Status" << "\n";
    cout << "-------------------------------------------------------------\n";
    for (int i = 0; i < total; i++) {
        cout << left << setw(30) << daftar[i].judul
             << setw(10) << daftar[i].durasi
             << setw(20) << daftar[i].status << "\n";
    }

    // Tanya apakah ingin mencari video
    char pilih;
    cout << "\nApakah Anda ingin mencari video berdasarkan judul? (y/t): ";
    cin >> pilih;

    if (pilih == 'y' || pilih == 'Y') {
        cin.ignore();
        char cari[40];
        cout << "Masukkan judul video yang ingin dicari: ";
        cin.getline(cari, 40);

        bool ditemukan = false;
        for (int i = 0; i < total; i++) {
            if (strcmp(daftar[i].judul, cari) == 0) {
                cout << "\nVideo ditemukan:\n";
                cout << left << setw(30) << "Judul"
                     << setw(10) << "Durasi"
                     << setw(20) << "Status" << "\n";
                cout << "-------------------------------------------------------------\n";
                cout << left << setw(30) << daftar[i].judul
                     << setw(10) << daftar[i].durasi
                     << setw(20) << daftar[i].status << "\n";
                ditemukan = true;
                break;
            }
        }

        if (!ditemukan) {
            cout << "\nVideo dengan judul \"" << cari << "\" tidak ditemukan.\n";
        }
    }
}

// Fungsi pencarian video di BST berdasarkan judul
Video* cariVideo(Video* node, const char* judul) {
    if (node == nullptr) return nullptr;

    int cmp = strcmp(judul, node->judul);
    if (cmp == 0) return node;
    else if (cmp < 0) return cariVideo(node->left, judul);
    else return cariVideo(node->right, judul);
}

// Playlist menggunakan array pointer (maksimal 100 video)
Video* playlist[100];
int jumlahPlaylist = 0;

void tambahKePlaylist() {
    if (root == nullptr) {
        cout << "Data video kosong. Silakan tambahkan video terlebih dahulu.\n";
        return;
    }

    char judulCari[40];
    cin.ignore(); // Buang newline sisa input sebelumnya
    cout << "Masukkan judul video yang ingin ditambahkan ke playlist: ";
    cin.getline(judulCari, 40);

    Video* target = cariVideo(root, judulCari);
    if (target == nullptr) {
        cout << "Video tidak ditemukan.\n";
        return;
    }

    if (strcmp(target->status, "Dalam Antrean") == 0 || strcmp(target->status, "Sedang Diputar") == 0) {
        cout << "Video ini sudah ada di playlist atau sedang diputar. Tidak bisa ditambahkan.\n";
        return;
    }

    if (jumlahPlaylist == 0) {
        strcpy(target->status, "Sedang Diputar");
    } else {
        strcpy(target->status, "Dalam Antrean");
    }

    playlist[jumlahPlaylist++] = target;
    cout << "Video berhasil ditambahkan ke playlist.\n";
}

// Jika kamu ingin tampilkan isi playlist:
void tampilkanPlaylist() {
    if (jumlahPlaylist == 0) {
        cout << "Playlist kosong.\n";
        return;
    }
    cout << "\nIsi Playlist:\n";
    cout << "-------------------------------------------------------------\n";
    cout << left << setw(30) << "Judul"
         << setw(10) << "Durasi"
         << setw(20) << "Status" << "\n";
    cout << "-------------------------------------------------------------\n";
    for (int i = 0; i < jumlahPlaylist; i++) {
        cout << left << setw(30) << playlist[i]->judul
             << setw(10) << playlist[i]->durasi
             << setw(20) << playlist[i]->status << "\n";
    }
}

const int MAX_RIWAYAT = 100;
Video* riwayatTontonan[MAX_RIWAYAT];
int jumlahRiwayat = 0;

void tontonVideo() {
    if (jumlahPlaylist == 0) {
        cout << "Playlist kosong. Tambahkan video ke playlist terlebih dahulu.\n";
        return;
    }

    // Ambil video yang sedang diputar
    Video* sedangDiputar = playlist[0];

    cout << "\nMemutar video: " << sedangDiputar->judul << "\n";
    cout << "Durasi: " << sedangDiputar->durasi << " menit\n";
    cout << "Status: Sedang Diputar\n";

    // Simulasikan selesai diputar
    strcpy(sedangDiputar->status, "Tersedia");

    // Tambahkan ke riwayat
    if (jumlahRiwayat < MAX_RIWAYAT) {
        riwayatTontonan[jumlahRiwayat++] = sedangDiputar;
    } else {
        cout << "Riwayat tontonan penuh. Video tidak disimpan ke riwayat.\n";
    }

    // Geser playlist ke kiri (hapus video yang diputar)
    for (int i = 1; i < jumlahPlaylist; i++) {
        playlist[i - 1] = playlist[i];
    }
    jumlahPlaylist--;

    // Set video berikutnya menjadi "Sedang Diputar"
    if (jumlahPlaylist > 0) {
        strcpy(playlist[0]->status, "Sedang Diputar");
    }

    cout << "Video selesai diputar dan masuk ke riwayat tontonan.\n";
}

void tampilkanRiwayat() {
    if (jumlahRiwayat == 0) {
        cout << "Riwayat tontonan kosong.\n";
        return;
    }

    cout << "\nRiwayat Tontonan (Terbaru di Atas):\n";
    cout << "-------------------------------------------------------------\n";
    cout << left << setw(30) << "Judul"
         << setw(10) << "Durasi"
         << setw(20) << "Status" << "\n";
    cout << "-------------------------------------------------------------\n";

    // Tampilkan dari yang terbaru (paling akhir ditonton)
    for (int i = jumlahRiwayat - 1; i >= 0; i--) {
        cout << left << setw(30) << riwayatTontonan[i]->judul
             << setw(10) << riwayatTontonan[i]->durasi
             << setw(20) << riwayatTontonan[i]->status << "\n";
    }
}

void hapusVideo() {
    if (root == nullptr) {
        cout << "Tidak ada video yang bisa dihapus.\n";
        return;
    }

    char judulHapus[40];
    cin.ignore();
    cout << "Masukkan judul video yang ingin dihapus: ";
    cin.getline(judulHapus, 40);

    // Cek apakah video sedang diputar atau dalam antrean
    bool adaDiPlaylist = false;
    bool sedangDiputar = false;

    for (int i = 0; i < jumlahPlaylist; i++) {
        if (strcmp(playlist[i]->judul, judulHapus) == 0) {
            adaDiPlaylist = true;
            if (i == 0 && strcmp(playlist[i]->status, "Sedang Diputar") == 0) {
                sedangDiputar = true;
            }
            break;
        }
    }

    // Kalau ada di playlist, minta konfirmasi
    if (adaDiPlaylist) {
        cout << "Video sedang ";
        if (sedangDiputar)
            cout << "diputar.\n";
        else
            cout << "dalam antrean.\n";
        cout << "Yakin ingin menghapus? (y/t): ";
        char konfirmasi;
        cin >> konfirmasi;
        if (konfirmasi != 'y' && konfirmasi != 'Y') {
            cout << "Penghapusan dibatalkan.\n";
            return;
        }
    }

    // Hapus dari playlist jika ada
    for (int i = 0; i < jumlahPlaylist; i++) {
        if (strcmp(playlist[i]->judul, judulHapus) == 0) {
            // Geser elemen playlist
            for (int j = i + 1; j < jumlahPlaylist; j++) {
                playlist[j - 1] = playlist[j];
            }
            jumlahPlaylist--;
            break;
        }
    }

    // Proses penghapusan dari BST
    Video* parent = nullptr;
    Video* current = root;

    while (current != nullptr && strcmp(current->judul, judulHapus) != 0) {
        parent = current;
        if (strcmp(judulHapus, current->judul) < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    if (current == nullptr) {
        cout << "Video tidak ditemukan.\n";
        return;
    }

    // Node dengan satu atau tidak ada anak
    if (current->left == nullptr || current->right == nullptr) {
        Video* newCurrent = (current->left != nullptr) ? current->left : current->right;

        if (parent == nullptr) {
            root = newCurrent;
        } else if (parent->left == current) {
            parent->left = newCurrent;
        } else {
            parent->right = newCurrent;
        }

        delete current;
    } else {
        // Node dengan dua anak
        Video* successorParent = current;
        Video* successor = current->right;

        while (successor->left != nullptr) {
            successorParent = successor;
            successor = successor->left;
        }

        // Salin data successor ke current
        strcpy(current->judul, successor->judul);
        current->durasi = successor->durasi;
        strcpy(current->status, successor->status);

        // Hapus successor dari tree
        if (successorParent->left == successor) {
            successorParent->left = successor->right;
        } else {
            successorParent->right = successor->right;
        }

        delete successor;
    }

    cout << "Video \"" << judulHapus << "\" berhasil dihapus.\n";

    // Update file
    latres = fopen("IDLIX_Tube.dat", "wb");
    if (latres != nullptr) {
        simpanKeFile(root, latres);
        fclose(latres);
    }
}

void undo() {
    if (strcmp(aksiTerakhir.tipe, "none") == 0) {
        cout << "Tidak ada aksi yang bisa di-undo.\n";
        return;
    }

    if (strcmp(aksiTerakhir.tipe, "tambah") == 0) {
        // Hapus video yang baru ditambahkan
        Video* v = aksiTerakhir.data;
        Video* dummy = buatNode(v->judul, v->durasi, v->status);
        insertVideo(root, dummy); // Agar bisa ditemukan
        hapusVideo(); // Pakai fungsi hapus yang sudah ada
        cout << "Undo: Tambah video dibatalkan.\n";
    } 
    else if (strcmp(aksiTerakhir.tipe, "hapus") == 0) {
        insertVideo(root, aksiTerakhir.data);
        cout << "Undo: Hapus video dibatalkan.\n";
    } 
    else if (strcmp(aksiTerakhir.tipe, "playlist") == 0) {
        if (jumlahPlaylist > 0) {
            jumlahPlaylist--;
            strcpy(aksiTerakhir.data->status, "Tersedia");
            cout << "Undo: Video dikeluarkan dari playlist.\n";
        }
    } 
    else if (strcmp(aksiTerakhir.tipe, "tonton") == 0) {
        jumlahPlaylist++;
        for (int i = jumlahPlaylist - 1; i > 0; i--) {
            playlist[i] = playlist[i - 1];
        }
        playlist[0] = aksiTerakhir.data;
        strcpy(aksiTerakhir.data->status, "Sedang Diputar");
        jumlahRiwayat--;
        cout << "Undo: Video yang ditonton dikembalikan ke playlist.\n";
    }

    // Reset aksi
    strcpy(aksiTerakhir.tipe, "none");
    aksiTerakhir.data = nullptr;
    aksiTerakhir.posisi = -1;
}

int main() {
    int pilihan;

    do {
        cout << "\n=== IDLIX Tube ===\n";
        cout << "1. Tambah Video Baru\n";
        cout << "2. Tampilkan Video\n";
        cout << "3. Tambah ke Playlist\n";
        cout << "4. Tonton Video\n";
        cout << "5. Tampilkan Playlist\n";
        cout << "6. Tampilkan Riwayat Tontonan\n";
        cout << "7. Hapus Video\n";
        cout << "8. Undo Aksi Terakhir\n";
        cout << "9. Keluar\n";
        cout << "Pilih menu: ";
        cin >> pilihan;

        switch (pilihan) {
            case 1:
                tambahVideo();
                break;
            case 2:
                tampilkanVideo();
                break;
            case 3:
                tambahKePlaylist();
                break;
            case 4:
                tontonVideo();
                break;
            case 5:
                tampilkanPlaylist();
                break;
            case 6:
                tampilkanRiwayat();
                break;
            case 7:
                hapusVideo();
                break;
            case 8:
                undo();
                break;
            case 9:
                cout << "Terima kasih telah menggunakan IDLIX Tube!\n";
                break;
            default:
                cout << "Pilihan tidak valid! Silakan coba lagi.\n";
        }
    } while (pilihan != 9);

    return 0;
}