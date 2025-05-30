#include <iostream>
        #include <stdio.h>
        #include <cstring>
        #include <iomanip>
        using namespace std;

        FILE *latres;

        struct Video {
            char judul[40];
            int durasi;
            char status[25];
            Video *left;
            Video *right;
        };

        struct Aksi {
            char tipe[20];           // "tambah", "hapus", "playlist", "tonton"
            Video* data;             // Pointer ke video terkait
            int posisi;              // Posisi dalam playlist/riwayat jika diperlukan
        };

        Aksi aksiTerakhir = {"none", nullptr, -1};

        // Root BST global
        Video *root = nullptr;

        // Playlist (array pointer)
        Video* playlist[100];
        int jumlahPlaylist = 0;

        // Riwayat tontonan (stack-like array pointer)
        const int MAX_RIWAYAT = 100;
        Video* riwayatTontonan[MAX_RIWAYAT];
        int jumlahRiwayat = 0;

        // Fungsi buat node video baru
        Video* buatNode(const char* judul, int durasi, const char* status) {
            Video* baru = new Video;
            strcpy(baru->judul, judul);
            baru->durasi = durasi;
            strcpy(baru->status, status);
            baru->left = nullptr;
            baru->right = nullptr;
            return baru;
        }

        // Fungsi insert video ke BST
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
                    delete baru;
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

        // Simpan node BST ke file (preorder), hanya data (tanpa pointer)
        void simpanKeFile(Video* node, FILE* file) {
            if (node == nullptr) return;
            fwrite(node, sizeof(Video) - sizeof(Video*) * 2, 1, file);
            simpanKeFile(node->left, file);
            simpanKeFile(node->right, file);
        }

        // Load video dari file ke BST
        void loadDariFile(Video *&root) {
            root = nullptr;
            latres = fopen("IDLIX_Tube.dat", "rb");
            if (latres == nullptr) {
                cout << "File data video tidak ditemukan, mulai dengan data kosong.\n";
                return;
            }

            Video temp;
            while (fread(&temp, sizeof(Video) - sizeof(Video*) * 2, 1, latres) == 1) {
                Video* baru = buatNode(temp.judul, temp.durasi, temp.status);
                insertVideo(root, baru);
            }

            fclose(latres);
        }

        // Fungsi tambah video baru dari input user
        void tambahVideo() {
            int jumlah;
            cout << "Masukkan jumlah video yang ingin ditambahkan: ";
            cin >> jumlah;
            if (jumlah <= 0) {
                cout << "Jumlah tidak valid!\n";
                return;
            }

            cin.ignore();
            for (int i = 0; i < jumlah; i++) {
                char judul[40];
                int durasi;
                char status[25];

                cout << "\nVideo ke-" << (i + 1) << ":\n";
                cout << "Judul Video : ";
                cin.getline(judul, 40);
                cout << "Durasi (menit) : ";
                cin >> durasi;
                cin.ignore();
                cout << "Status (Tersedia / Dalam Antrean / Sedang Diputar): ";
                cin.getline(status, 25);

                Video *baru = buatNode(judul, durasi, status);
                insertVideo(root, baru);

                // Simpan aksi untuk undo
                strcpy(aksiTerakhir.tipe, "tambah");
                aksiTerakhir.data = baru;
            }

            // Simpan seluruh BST ke file (overwrite)
            latres = fopen("IDLIX_Tube.dat", "wb");
            if (latres == nullptr) {
                cout << "Gagal membuka file untuk penyimpanan.\n";
                return;
            }
            simpanKeFile(root, latres);
            fclose(latres);

            cout << "\nSemua video berhasil disimpan ke file.\n";
        }

        // Tampilkan daftar video dari file, urut berdasarkan judul
        void tampilkanVideo() {
            latres = fopen("IDLIX_Tube.dat", "rb");
            if (latres == nullptr) {
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

            // Urutkan dengan bubble sort berdasarkan judul A-Z
            for (int i = 0; i < total - 1; i++) {
                for (int j = i + 1; j < total; j++) {
                    if (strcmp(daftar[i].judul, daftar[j].judul) > 0) {
                        Video temp = daftar[i];
                        daftar[i] = daftar[j];
                        daftar[j] = temp;
                    }
                }
            }

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

            // Opsi pencarian video berdasarkan judul
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

        // Cari video dalam BST berdasarkan judul
        Video* cariVideo(Video* node, const char* judul) {
            if (node == nullptr) return nullptr;
            int cmp = strcmp(judul, node->judul);
            if (cmp == 0) return node;
            else if (cmp < 0) return cariVideo(node->left, judul);
            else return cariVideo(node->right, judul);
        }

        // Tambah video ke playlist
        void tambahKePlaylist() {
            if (root == nullptr) {
                cout << "Data video kosong. Silakan tambahkan video terlebih dahulu.\n";
                return;
            }

            char judulCari[40];
            cin.ignore();
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

            // Simpan aksi undo
            strcpy(aksiTerakhir.tipe, "playlist");
            aksiTerakhir.data = target;
        }

        // Tampilkan isi playlist
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

        // Fungsi tonton video (playlist paling depan)
        void tontonVideo() {
            if (jumlahPlaylist == 0) {
                cout << "Playlist kosong. Tambahkan video ke playlist terlebih dahulu.\n";
                return;
            }

            Video* sedangDiputar = playlist[0];

            cout << "\nMemutar video: " << sedangDiputar->judul << "\n";
            cout << "Durasi: " << sedangDiputar->durasi << " menit\n";
            cout << "Status: Sedang Diputar\n";

            // Set status video selesai diputar
            strcpy(sedangDiputar->status, "Tersedia");

            // Simpan aksi undo
            strcpy(aksiTerakhir.tipe, "tonton");
            aksiTerakhir.data = sedangDiputar;

            // Tambah ke riwayat tontonan
            if (jumlahRiwayat < MAX_RIWAYAT) {
                riwayatTontonan[jumlahRiwayat++] = sedangDiputar;
            } else {
                cout << "Riwayat tontonan penuh.\n";
            }

            // Hapus video dari playlist dan geser
            for (int i = 1; i < jumlahPlaylist; i++) {
                playlist[i - 1] = playlist[i];
            }
            jumlahPlaylist--;

            // Jika masih ada video di playlist, update status video pertama menjadi "Sedang Diputar"
            if (jumlahPlaylist > 0) {
                strcpy(playlist[0]->status, "Sedang Diputar");
            }

            // Simpan ke file
            latres = fopen("IDLIX_Tube.dat", "wb");
            if (latres != nullptr) {
                simpanKeFile(root, latres);
                fclose(latres);
            }
        }

        // Tampilkan riwayat tontonan
        void tampilkanRiwayat() {
            if (jumlahRiwayat == 0) {
                cout << "Riwayat tontonan kosong.\n";
                return;
            }

            cout << "\nRiwayat Tontonan:\n";
            cout << "-------------------------------------------------------------\n";
            cout << left << setw(30) << "Judul"
                << setw(10) << "Durasi"
                << setw(20) << "Status" << "\n";
            cout << "-------------------------------------------------------------\n";

            for (int i = jumlahRiwayat - 1; i >= 0; i--) {
                cout << left << setw(30) << riwayatTontonan[i]->judul
                    << setw(10) << riwayatTontonan[i]->durasi
                    << setw(20) << riwayatTontonan[i]->status << "\n";
            }
        }

        // Fungsi bantu untuk insert semua video kecuali yang dihapus
        void salinKecuali(Video* asal, Video*& tujuan, const char* judulKecuali) {
             if (asal == nullptr) return;
             if (strcmp(asal->judul, judulKecuali) != 0) {
                        Video* v = buatNode(asal->judul, asal->durasi, asal->status);
                        insertVideo(tujuan, v);
    }
    salinKecuali(asal->left, tujuan, judulKecuali);
    salinKecuali(asal->right, tujuan, judulKecuali);
};

void hapusVideo() {
    if (root == nullptr) {
        cout << "Tidak ada video yang tersedia.\n";
        return;
    }

    cin.ignore();
    char judulHapus[40];
    cout << "Masukkan judul video yang ingin dihapus: ";
    cin.getline(judulHapus, 40);

    Video* target = cariVideo(root, judulHapus);
    if (target == nullptr) {
        cout << "Video tidak ditemukan.\n";
        return;
    }

    // Periksa apakah sedang diputar atau dalam antrean
    if (strcmp(target->status, "Sedang Diputar") == 0 || strcmp(target->status, "Dalam Antrean") == 0) {
        cout << "Video yang ingin dihapus sedang " << target->status << ". Yakin untuk tetap menghapus? (y/t): ";
        char konfirmasi;
        cin >> konfirmasi;
        if (konfirmasi != 'y' && konfirmasi != 'Y') {
            cout << "Penghapusan dibatalkan.\n";
            return;
        }
    }

    // Hapus dari playlist jika ada
    for (int i = 0; i < jumlahPlaylist; i++) {
        if (playlist[i] == target) {
            for (int j = i + 1; j < jumlahPlaylist; j++) {
                playlist[j - 1] = playlist[j];
            }
            jumlahPlaylist--;
            break;
        }
    }

    // Buat BST baru tanpa video yang dihapus
    Video* newRoot = nullptr;
    salinKecuali(root, newRoot, judulHapus);
    root = newRoot;

    // Simpan ke file
    latres = fopen("IDLIX_Tube.dat", "wb");
    if (latres != nullptr) {
        simpanKeFile(root, latres);
        fclose(latres);
    }

    // Simpan aksi untuk undo
    strcpy(aksiTerakhir.tipe, "hapus");
    aksiTerakhir.data = target;

    cout << "Video \"" << judulHapus << "\" berhasil dihapus dari sistem.\n";
}

        // Undo aksi terakhir
        void undo() {
            if (strcmp(aksiTerakhir.tipe, "none") == 0) {
                cout << "Tidak ada aksi yang bisa di-undo.\n";
                return;
            }

            if (strcmp(aksiTerakhir.tipe, "tambah") == 0) {
                // Undo tambah video = hapus video dari BST
                // Implementasi hapus node BST agak kompleks, maka kita rebuild BST tanpa video tsb
                cout << "Undo tambah video: " << aksiTerakhir.data->judul << "\n";

                // Buat BST baru tanpa video tsb
                Video* newRoot = nullptr;

                // Simpan ke file
                latres = fopen("IDLIX_Tube.dat", "wb");
                if (latres != nullptr) {
                    simpanKeFile(root, latres);
                    fclose(latres);
                }

                cout << "Undo berhasil, video dihapus.\n";
            } else if (strcmp(aksiTerakhir.tipe, "playlist") == 0) {
                // Undo tambah ke playlist = hapus dari playlist dan update status
                cout << "Undo tambah ke playlist video: " << aksiTerakhir.data->judul << "\n";

                // Cari posisi video di playlist
                int idx = -1;
                for (int i = 0; i < jumlahPlaylist; i++) {
                    if (playlist[i] == aksiTerakhir.data) {
                        idx = i;
                        break;
                    }
                }
                if (idx != -1) {
                    // Hapus dari playlist geser ke kiri
                    for (int i = idx + 1; i < jumlahPlaylist; i++) {
                        playlist[i - 1] = playlist[i];
                    }
                    jumlahPlaylist--;

                    // Ubah status video jadi "Tersedia"
                    strcpy(aksiTerakhir.data->status, "Tersedia");

                    // Jika video pertama di playlist, update status video baru jadi "Sedang Diputar"
                    if (jumlahPlaylist > 0) {
                        strcpy(playlist[0]->status, "Sedang Diputar");
                    }

                    cout << "Undo berhasil, video dihapus dari playlist.\n";
                } else {
                    cout << "Video tidak ditemukan di playlist.\n";
                }
            } else if (strcmp(aksiTerakhir.tipe, "tonton") == 0) {
                // Undo tonton video = hapus video terakhir di riwayat, masukkan kembali ke playlist di depan
                cout << "Undo tonton video: " << aksiTerakhir.data->judul << "\n";

                if (jumlahRiwayat > 0 && riwayatTontonan[jumlahRiwayat - 1] == aksiTerakhir.data) {
                    jumlahRiwayat--;

                    // Geser playlist kanan semua
                    for (int i = jumlahPlaylist; i > 0; i--) {
                        playlist[i] = playlist[i - 1];
                    }
                    playlist[0] = aksiTerakhir.data;
                    jumlahPlaylist++;

                    // Update status video
                    strcpy(aksiTerakhir.data->status, "Sedang Diputar");

                    // Update status video kedua playlist (jika ada) jadi "Dalam Antrean"
                    if (jumlahPlaylist > 1) {
                        strcpy(playlist[1]->status, "Dalam Antrean");
                    }

                    cout << "Undo berhasil, video dikembalikan ke playlist.\n";
                } else {
                    cout << "Tidak ada riwayat tontonan untuk di-undo.\n";
                }
            }

            // Reset aksi terakhir
            strcpy(aksiTerakhir.tipe, "none");
            aksiTerakhir.data = nullptr;
            aksiTerakhir.posisi = -1;
        }

        void menu() {
            int pilihan;
            do {
                cout << "\n=== IDLIX Tube ===\n";
                cout << "1. Tambah Video\n";
                cout << "2. Tampilkan Video\n";
                cout << "3. Tambah Video ke Playlist\n";
                cout << "4. Tampilkan Playlist\n";
                cout << "5. Tonton Video (Putar Playlist)\n";
                cout << "6. Hapus Video\n";
                cout << "7. Tampilkan Riwayat Tontonan\n";
                cout << "8. Undo Aksi Terakhir\n";
                cout << "0. Keluar\n";
                cout << "Pilih menu: ";
                cin >> pilihan;

                switch (pilihan) {
                    case 1: tambahVideo(); break;
                    case 2: tampilkanVideo(); break;
                    case 3: tambahKePlaylist(); break;
                    case 4: tampilkanPlaylist(); break;
                    case 5: tontonVideo(); break;
                    case 6: hapusVideo(); break;
                    case 7: tampilkanRiwayat(); break;
                    case 8: undo(); break;
                    case 0: cout << "Terima kasih telah menggunakan IDLIX Tube.\n"; break;
                    default: cout << "Pilihan tidak valid!\n"; break;
                }
            } while (pilihan != 0);
        }



        int main() {
    loadDariFile(root);
    menu();

    return 0;
}