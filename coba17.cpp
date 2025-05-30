#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cstdio>
using namespace std;

const int MAX_KURSI = 100;
const int MAX_ULASAN = 1000;
const int MAX_FILM = 10;
const int MAX_JADWAL = 5;
const int MAX_MAKANAN = 10;
const int BARIS_KURSI = 6;
const int KOLOM_KURSI = 6;
const int MAX_PESANAN_MAKANAN = 10;

// Deklarasi fungsi lebih dulu agar tidak error
struct User;
struct Pesanan;
struct Film;
void tambahUser(User* u);
void tambahPesanan(Pesanan* p);
Film* cariFilmById(int id);

struct Film {
    int id;
    string judul;
    int stok;
    int jumlahJadwal;
    string jadwal[MAX_JADWAL];
    int durasi;
    int terjual;
    string kursiTerisi[MAX_KURSI];
    int jumlahKursiTerisi;
    float rating[MAX_ULASAN];
    char ulasan[MAX_ULASAN][1000];
    int jumlahUlasan;
};
struct Makanan {
    int id;
    string nama;
    int harga;
};
struct Pesanan {
    string username;
    int filmId;
    string jadwal;
    string kursiDipilih;
    int makananId[MAX_PESANAN_MAKANAN];
    int makananJumlah[MAX_PESANAN_MAKANAN];
    int totalMakanan;
    int totalHarga;
    Pesanan* next;
};
struct User {
    string username;
    string password;
    bool isAdmin;
    User* prev;
    User* next;
};

Film films[MAX_FILM];
Makanan makanan[MAX_MAKANAN];
Pesanan* pesananHead = nullptr;
User* userHead = nullptr;
User* loggedInUser = nullptr;

// === FILE PERSISTENCE ===
void saveUsers() {
    FILE* f = fopen("users.txt", "w");
    User* cur = userHead;
    while (cur) {
        fprintf(f, "%s|%s|%d\n", cur->username.c_str(), cur->password.c_str(), cur->isAdmin ? 1 : 0);
        cur = cur->next;
    }
    fclose(f);
}
void loadUsers() {
    FILE* f = fopen("users.txt", "r");
    if (!f) return;
    char uname[100], pass[100]; int admin;
    while (fscanf(f, "%99[^|]|%99[^|]|%d\n", uname, pass, &admin) == 3) {
        User* u = new User;
        u->username = uname;
        u->password = pass;
        u->isAdmin = admin;
        u->prev = u->next = nullptr;
        tambahUser(u);
    }
    fclose(f);
}
void saveFilms() {
    FILE* f = fopen("films.txt", "w");
    for (int i = 0; i < MAX_FILM; i++) {
        if (films[i].id == 0) continue;
        fprintf(f, "%d|%s|%d|%d|", films[i].id, films[i].judul.c_str(), films[i].stok, films[i].durasi);
        for (int j = 0; j < films[i].jumlahJadwal; j++) {
            fprintf(f, "%s,", films[i].jadwal[j].c_str());
        }
        fprintf(f, "|%d|%d|", films[i].jumlahJadwal, films[i].terjual);
        for (int j = 0; j < films[i].jumlahKursiTerisi; j++) {
            fprintf(f, "%s,", films[i].kursiTerisi[j].c_str());
        }
        fprintf(f, "|%d\n", films[i].jumlahKursiTerisi);
    }
    fclose(f);
}
void loadFilms() {
    FILE* f = fopen("films.txt", "r");
    if (!f) return;
    char judul[256], jadwalStr[256], kursiStr[256];
    int id, stok, durasi, jumlahJadwal, terjual, jumlahKursiTerisi;
    while (fscanf(f, "%d|%255[^|]|%d|%d|%255[^|]|%d|%d|%255[^|]|%d\n",
        &id, judul, &stok, &durasi, jadwalStr, &jumlahJadwal, &terjual, kursiStr, &jumlahKursiTerisi) == 9) {
        Film& film = films[id-1];
        film.id = id;
        film.judul = judul;
        film.stok = stok;
        film.durasi = durasi;
        film.terjual = terjual;
        // Parse jadwal
        string sjadwal = jadwalStr;
        int j = 0;
        stringstream ssj(sjadwal);
        string jad;
        while (getline(ssj, jad, ',') && j < MAX_JADWAL) {
            if (!jad.empty()) film.jadwal[j++] = jad;
        }
        film.jumlahJadwal = jumlahJadwal;
        // Parse kursi terisi
        film.jumlahKursiTerisi = 0;
        string skursi = kursiStr;
        stringstream ssk(skursi);
        string kursi;
        while (getline(ssk, kursi, ',') && film.jumlahKursiTerisi < MAX_KURSI) {
            if (!kursi.empty()) film.kursiTerisi[film.jumlahKursiTerisi++] = kursi;
        }
    }
    fclose(f);
}
void savePesanan() {
    FILE* f = fopen("pesanan.txt", "w");
    Pesanan* cur = pesananHead;
    while (cur) {
        fprintf(f, "%s|%d|%s|%s|", cur->username.c_str(), cur->filmId, cur->jadwal.c_str(), cur->kursiDipilih.c_str());
        for (int i = 0; i < cur->totalMakanan; i++)
            fprintf(f, "%d:%d,", cur->makananId[i], cur->makananJumlah[i]);
        fprintf(f, "|%d|%d\n", cur->totalMakanan, cur->totalHarga);
        cur = cur->next;
    }
    fclose(f);
}
void loadPesanan() {
    FILE* f = fopen("pesanan.txt", "r");
    if (!f) return;
    char uname[100], jadwal[100], kursi[200], makananStr[300];
    int filmId, totalMakanan, totalHarga;
    while (fscanf(f, "%99[^|]|%d|%99[^|]|%199[^|]|%299[^|]|%d|%d\n", uname, &filmId, jadwal, kursi, makananStr, &totalMakanan, &totalHarga) == 7) {
        Pesanan* p = new Pesanan;
        p->username = uname;
        p->filmId = filmId;
        p->jadwal = jadwal;
        p->kursiDipilih = kursi;
        p->totalMakanan = totalMakanan;
        p->totalHarga = totalHarga;
        p->next = nullptr;
        // parse makanan
        string smakanan = makananStr;
        stringstream ssm(smakanan);
        string m;
        int i = 0;
        while (getline(ssm, m, ',') && i < MAX_PESANAN_MAKANAN) {
            size_t titik2 = m.find(":");
            if (titik2 != string::npos) {
                p->makananId[i] = atoi(m.substr(0, titik2).c_str());
                p->makananJumlah[i] = atoi(m.substr(titik2+1).c_str());
                i++;
            }
        }
        tambahPesanan(p);
        // update kursi film
        Film* ffilm = cariFilmById(filmId);
        if (ffilm) {
            istringstream iss(p->kursiDipilih);
            string k;
            while (iss >> k)
                ffilm->kursiTerisi[ffilm->jumlahKursiTerisi++] = k;
        }
    }
    fclose(f);
}
// -- END FILE PERSISTENCE ---

void pause() {
    cout << "\nTekan ENTER untuk lanjut...";
    cin.clear(); cin.ignore(10000, '\n'); cin.get();
}

void clearScreen() {
#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#else
    system("clear");
#endif
}

void header(const string& title) {
    clearScreen();
    cout << "==============================================\n";
    cout << "       BIOSKOP XXI - " << title << "\n";
    cout << "==============================================\n\n";
}
void pauseClear() { pause(); clearScreen(); }

bool isKursiValid(const string& kursi) {
    if (kursi.length() < 2) return false;
    char row = toupper(kursi[0]);
    if (row < 'A' || row >= 'A' + BARIS_KURSI) return false;
    try {
        int col = stoi(kursi.substr(1));
        return col >= 1 && col <= KOLOM_KURSI;
    } catch (...) { return false; }
}
Film* cariFilmById(int id) {
    for (int i = 0; i < MAX_FILM; i++)
        if (films[i].id == id) return &films[i];
    return nullptr;
}
Makanan* cariMakananById(int id) {
    for (int i = 0; i < MAX_MAKANAN; i++)
        if (makanan[i].id == id) return &makanan[i];
    return nullptr;
}
User* cariUser(const string& username) {
    User* cur = userHead;
    while (cur) {
        if (cur->username == username) return cur;
        cur = cur->next;
    }
    return nullptr;
}
void tambahUser(User* u) {
    u->prev = nullptr; u->next = nullptr;
    if (!userHead) userHead = u;
    else {
        User* cur = userHead;
        while (cur->next) cur = cur->next;
        cur->next = u; u->prev = cur;
    }
}
void tambahPesanan(Pesanan* p) {
    p->next = nullptr;
    if (!pesananHead) pesananHead = p;
    else {
        Pesanan* cur = pesananHead;
        while (cur->next) cur = cur->next;
        cur->next = p;
    }
}
void tampilkanDenahKursi(const Film& film) {
    cout << "\nDenah Kursi (X = Terisi, O = Kosong):\n   ";
    for (int col = 1; col <= KOLOM_KURSI; ++col) cout << setw(3) << col;
    cout << "\n";
    for (char row = 'A'; row < 'A' + BARIS_KURSI; ++row) {
        cout << row << "  ";
        for (int col = 1; col <= KOLOM_KURSI; ++col) {
            string kursi = string(1, row) + to_string(col);
            bool terisi = false;
            for (int i = 0; i < film.jumlahKursiTerisi; ++i)
                if (film.kursiTerisi[i] == kursi) terisi = true;
            cout << setw(3) << (terisi ? "X" : "O");
        }
        cout << "\n";
    }
    cout << "\n";
}
void pilihKursi(Film& film, string kursiTerpilih[], int& jumlahDipilih, int jumlahTiket) {
    jumlahDipilih = 0;
    while (jumlahDipilih < jumlahTiket) {
        tampilkanDenahKursi(film);
        cout << "Pilih kursi ke-" << (jumlahDipilih + 1) << " (misal: A1): ";
        string kursi; cin >> kursi;
        if (!isKursiValid(kursi)) { cout << "Format kursi salah.\n"; continue; }
        bool sudahDiambil = false;
        for (int i = 0; i < film.jumlahKursiTerisi; ++i)
            if (film.kursiTerisi[i] == kursi) sudahDiambil = true;
        for (int i = 0; i < jumlahDipilih; ++i)
            if (kursiTerpilih[i] == kursi) sudahDiambil = true;
        if (sudahDiambil) { cout << "Kursi sudah diambil.\n"; continue; }
        kursiTerpilih[jumlahDipilih++] = kursi;
    }
}
void tabelFilm() {
    cout << "+-----+-------------------------+------+----------------------+--------+----------+\n";
    cout << "| ID  | Judul                   | Stok | Jadwal               | Durasi | Terjual  |\n";
    cout << "+-----+-------------------------+------+----------------------+--------+----------+\n";
    for (int i = 0; i < MAX_FILM; i++) {
        if (films[i].id == 0) continue;
        string jadwalStr = "";
        for (int j = 0; j < films[i].jumlahJadwal; ++j)
            jadwalStr += films[i].jadwal[j] + " ";
        cout << "| " << setw(3) << films[i].id << " | "
             << left << setw(23) << films[i].judul << " | "
             << right << setw(4) << films[i].stok << " | "
             << left << setw(20) << jadwalStr << " | "
             << right << setw(6) << films[i].durasi << " | "
             << right << setw(8) << films[i].terjual << " |\n";
    }
    cout << "+-----+-------------------------+------+----------------------+--------+----------+\n";
}
void tabelMakanan() {
    cout << "+-----+----------------------+-----------+\n";
    cout << "| ID  | Nama                 | Harga     |\n";
    cout << "+-----+----------------------+-----------+\n";
    for (int i = 0; i < MAX_MAKANAN; i++) {
        if (makanan[i].id == 0) continue;
        cout << "| " << setw(3) << makanan[i].id << " | "
             << left << setw(20) << makanan[i].nama << " | "
             << right << "Rp " << setw(7) << makanan[i].harga << " |\n";
    }
    cout << "+-----+----------------------+-----------+\n";
}
void initData() {
    string jadwalF[MAX_FILM][3] = {
        {"10:00","14:00","18:00"}, {"12:00","16:00","20:00"}, {"11:00","15:00","19:00"},
        {"13:00","17:00","21:00"}, {"10:30","14:30","18:30"}, {"11:30","15:30","19:30"},
        {"12:30","16:30","20:30"}, {"10:15","14:15","18:15"}, {"13:15","17:15","21:15"}, {"11:45","15:45","19:45"}
    };
    string judulF[MAX_FILM] = {
        "Avengers: Endgame", "Joker", "Inception", "Parasite", "Interstellar",
        "The Dark Knight", "Titanic", "Frozen II", "The Matrix", "Avatar"
    };
    int stokF[MAX_FILM] = {100,80,90,70,75,60,50,65,55,85};
    int durasiF[MAX_FILM] = {181,122,148,132,169,152,195,103,136,162};
    for (int i = 0; i < MAX_FILM; i++) {
        films[i].id = i+1; films[i].judul = judulF[i]; films[i].stok = stokF[i];
        films[i].jumlahJadwal = 3; films[i].durasi = durasiF[i]; films[i].terjual = 0;
        films[i].jumlahKursiTerisi = films[i].jumlahUlasan = 0;
        for (int j = 0; j < 3; j++) films[i].jadwal[j] = jadwalF[i][j];
    }
    string namaM[MAX_MAKANAN] = {"Popcorn","Nachos","Hotdog","Soda","Water","Candy","Burger","Fries","Ice Cream","Coffee"};
    int hargaM[MAX_MAKANAN] = {15000,20000,25000,10000,8000,12000,30000,18000,22000,15000};
    for (int i = 0; i < MAX_MAKANAN; i++) {
        makanan[i].id = i+1; makanan[i].nama = namaM[i]; makanan[i].harga = hargaM[i];
    }
}

bool login() {
    header("Login");
    string uname, pass;
    cout << "Username: "; cin >> uname;
    cout << "Password: "; cin >> pass;
    User* u = cariUser(uname);
    if (u && u->password == pass) {
        loggedInUser = u; return true;
    } else {
        cout << "\nLogin gagal! Username/password salah.\n";
        pause(); return false;
    }
}
void daftar() {
    header("Daftar Akun Baru");
    string uname, pass, pass2;
    cout << "Masukkan username: "; cin >> uname;
    if (cariUser(uname)) { cout << "Username sudah terdaftar!\n"; pause(); return; }
    cout << "Masukkan password: "; cin >> pass;
    cout << "Konfirmasi password: "; cin >> pass2;
    if (pass != pass2) { cout << "Password tidak sama!\n"; pause(); return; }
    User* u = new User;
    u->username = uname; u->password = pass; u->isAdmin = false;
    tambahUser(u);
    saveUsers();
    cout << "Pendaftaran berhasil! Silakan login.\n";
    pause();
}
void logout() { loggedInUser = nullptr; cout << "Logout berhasil.\n"; pause(); }

void cariFilm() {
    header("Pencarian Film");
    cout << "Masukkan judul (atau sebagian judul) film: ";
    cin.ignore();
    string keyword; getline(cin, keyword);
    for (int i = 0; i < keyword.length(); ++i)
        if (keyword[i] >= 'A' && keyword[i] <= 'Z') keyword[i] = keyword[i] + 32;
    bool ditemukan = false;
    cout << "+-----+-------------------------+------+----------------------+--------+----------+\n";
    cout << "| ID  | Judul                   | Stok | Jadwal               | Durasi | Terjual  |\n";
    cout << "+-----+-------------------------+------+----------------------+--------+----------+\n";
    for (int i = 0; i < MAX_FILM; i++) {
        if (films[i].id == 0) continue;
        string judul = films[i].judul;
        for (int j = 0; j < judul.length(); ++j)
            if (judul[j] >= 'A' && judul[j] <= 'Z') judul[j] = judul[j] + 32;
        if (judul.find(keyword) != string::npos) {
            ditemukan = true;
            string jadwalStr = "";
            for (int j = 0; j < films[i].jumlahJadwal; ++j)
                if (!films[i].jadwal[j].empty()) jadwalStr += films[i].jadwal[j] + " ";
            cout << "| " << setw(3) << films[i].id << " | "
                 << setw(23) << left << films[i].judul << " | "
                 << setw(4) << films[i].stok << " | "
                 << setw(20) << jadwalStr << " | "
                 << setw(6) << films[i].durasi << " | "
                 << setw(8) << films[i].terjual << " |\n";
        }
    }
    if (!ditemukan)
        cout << "|              Tidak ditemukan film sesuai kata kunci!              |\n";
    cout << "+-----+-------------------------+------+----------------------+--------+----------+\n";
    pauseClear();
}

void tampilRatingUlasan(const Film& f) {
    if (f.jumlahUlasan == 0) {
        cout << "Belum ada rating/ulasan untuk film ini.\n";
        return;
    }
    float sum = 0;
    for (int i = 0; i < f.jumlahUlasan; ++i) sum += f.rating[i];
    cout << "Rata-rata rating: " << fixed << setprecision(1) << (sum / f.jumlahUlasan)
         << " dari " << f.jumlahUlasan << " ulasan\n";
    cout << "Daftar Ulasan:\n";
    for (int i = 0; i < f.jumlahUlasan; ++i)
        if (strlen(f.ulasan[i]) > 0) cout << "- " << f.ulasan[i] << "\n";
}

void isiRatingUlasan() {
    header("Rating & Ulasan Film");
    tabelFilm();
    cout << "Pilih ID film yang ingin diberi rating/ulasan: ";
    int id; cin >> id;
    Film* f = cariFilmById(id);
    if (!f) { cout << "Film tidak ditemukan.\n"; pauseClear(); return; }
    if (f->jumlahUlasan >= MAX_ULASAN) {
        cout << "Maaf, jumlah ulasan untuk film ini sudah penuh.\n";
        pauseClear(); return;
    }
    int nilai;
    do {
        cout << "Beri rating (1-5): "; cin >> nilai;
    } while (nilai < 1 || nilai > 5);
    cin.ignore();
    cout << "Tulis ulasan (opsional): ";
    string ulas; getline(cin, ulas);
    f->rating[f->jumlahUlasan] = nilai;
    strncpy(f->ulasan[f->jumlahUlasan], ulas.c_str(), sizeof(f->ulasan[f->jumlahUlasan]) - 1);
    f->ulasan[f->jumlahUlasan][sizeof(f->ulasan[f->jumlahUlasan]) - 1] = '\0';
    f->jumlahUlasan++;
    cout << "Terima kasih atas rating & ulasannya!\n";
    pauseClear();
}

string pilihJadwal(Film& film) {
    cout << "Pilih Jadwal:\n";
    int jumlahPilihan = 0;
    for (int i = 0; i < MAX_JADWAL; ++i)
        if (!film.jadwal[i].empty()) {
            cout << (jumlahPilihan + 1) << ". " << film.jadwal[i] << "\n";
            jumlahPilihan++;
        }
    int pil;
    do {
        cout << "Masukkan pilihan: "; cin >> pil;
    } while (pil < 1 || pil > jumlahPilihan);
    return film.jadwal[pil - 1];
}

void pilihMakanan(int makananId[], int makananJumlah[], int& jumlahPesanan) {
    jumlahPesanan = 0;
    char lagi = 'y';
    while ((lagi == 'y' || lagi == 'Y') && jumlahPesanan < MAX_PESANAN_MAKANAN) {
        tabelMakanan();
        cout << "Pilih ID makanan/minuman (0 untuk selesai): ";
        int id; cin >> id;
        if (id == 0) break;
        Makanan* m = cariMakananById(id);
        if (m) {
            cout << "Jumlah: "; int jml; cin >> jml;
            if (jml > 0) {
                makananId[jumlahPesanan] = id;
                makananJumlah[jumlahPesanan] = jml;
                jumlahPesanan++;
            } else cout << "Jumlah harus lebih dari 0.\n";
        } else cout << "Makanan tidak ditemukan.\n";
        if (jumlahPesanan < MAX_PESANAN_MAKANAN) {
            cout << "Tambah makanan lain? (y/n): ";
            cin >> lagi;
        } else {
            cout << "Maksimal jumlah pesanan makanan tercapai.\n"; break;
        }
    }
}

int hitungTotalHarga(Film& film, int jumlahTiket, const int makananId[], const int makananJumlah[], int jumlahPesanan) {
    int total = jumlahTiket * 50000;
    for (int i = 0; i < jumlahPesanan; i++) {
        Makanan* m = cariMakananById(makananId[i]);
        if (m) total += m->harga * makananJumlah[i];
    }
    return total;
}

void tampilRingkasanPesanan(Film* f, const string& jadwal,
    const string kursiTerpilih[], int jumlahKursi,
    const int makananId[], const int makananJumlah[], int jumlahMakanan,
    int totalHarga, int diskon = 0) {
    cout << "\n-----------------------------------------\n";
    cout << "|          RINGKASAN PESANAN            |\n";
    cout << "-----------------------------------------\n";
    cout << left << setw(15) << "| Film" << "| " << setw(22) << f->judul << " |\n";
    cout << left << setw(15) << "| Jadwal" << "| " << setw(22) << jadwal << " |\n";
    string kursiStr;
    for (int i = 0; i < jumlahKursi; ++i) kursiStr += kursiTerpilih[i] + " ";
    cout << left << setw(15) << "| Kursi" << "| " << setw(22) << kursiStr << " |\n";
    if (jumlahMakanan == 0) {
        cout << left << setw(15) << "| Makanan" << "| " << setw(22) << "-" << " |\n";
    } else {
        bool first = true;
        for (int i = 0; i < jumlahMakanan; ++i) {
            Makanan* m = cariMakananById(makananId[i]);
            if (m) {
                string item = m->nama + " x" + to_string(makananJumlah[i]);
                if (first) {
                    cout << left << setw(15) << "| Makanan" << "| " << setw(22) << item << " |\n";
                    first = false;
                } else {
                    cout << left << setw(15) << "|" << "| " << setw(22) << item << " |\n";
                }
            }
        }
    }
    if (diskon > 0)
        cout << left << setw(15) << "| Diskon" << "| Rp " << setw(19) << diskon << " |\n";
    cout << "----------------------------------------\n";
    cout << left << setw(15) << "| Total Harga" << "| Rp " << setw(19) << totalHarga << " |\n";
    cout << "----------------------------------------\n";
}

void prosesPembayaran(int& totalHarga) {
    cout << "\nApakah punya kode diskon? (y/n): ";
    char adaDiskon; cin >> adaDiskon;
    int diskon = 0;
    if (adaDiskon == 'y' || adaDiskon == 'Y') {
        cout << "Masukkan kode diskon: "; string kode; cin >> kode;
        if (kode == "DISKON10") {
            diskon = totalHarga * 0.1;
            totalHarga -= diskon;
            cout << "Kode diskon benar! Potongan 10%: Rp " << diskon << endl;
        } else cout << "Kode diskon salah atau tidak berlaku.\n";
    }
    cout << "\nTotal yang harus dibayar: Rp " << totalHarga << endl;
    int uangDibayar;
    do {
        cout << "Masukkan jumlah uang: Rp "; cin >> uangDibayar;
        if (uangDibayar < totalHarga)
            cout << "Uang tidak cukup. Kurang Rp " << (totalHarga - uangDibayar) << endl;
    } while (uangDibayar < totalHarga);
    if (uangDibayar > totalHarga)
        cout << "Kembalian: Rp " << (uangDibayar - totalHarga) << endl;
    cout << "Terima kasih atas pembayarannya!\n";
}

void prosesMakananDanRingkasan(
    Film& film,
    const string& jadwal,
    const string kursiTerpilih[],
    int jumlahKursi,
    int& totalHarga,
    int jumlahTiket,
    int makananId[],
    int makananJumlah[],
    int& totalMakanan
) {
    pilihMakanan(makananId, makananJumlah, totalMakanan);
    totalHarga = hitungTotalHarga(film, jumlahTiket, makananId, makananJumlah, totalMakanan);
    tampilRingkasanPesanan(&film, jadwal, kursiTerpilih, jumlahKursi, makananId, makananJumlah, totalMakanan, totalHarga);
}

void pesanTiket() {
    header("Pesan Tiket");
    cout << "Halo, " << loggedInUser->username << "\n\n";
    cout << "Menu User\n";
    cout << "----------------------------------------\n";
    tabelFilm();
    int idFilm;
    cout << "\nMasukkan ID film yang ingin dipesan: ";
    cin >> idFilm;
    Film* f = cariFilmById(idFilm);
    if (!f) { cout << "Film tidak ditemukan.\n"; pauseClear(); return; }
    if (f->stok <= 0) { cout << "Maaf, kursi penuh.\n"; pauseClear(); return; }
    string jadwal = pilihJadwal(*f);
    cout << "Berapa tiket yang ingin dipesan? "; int jumlahTiket; cin >> jumlahTiket;
    if (jumlahTiket > f->stok || jumlahTiket <= 0) {
        cout << "Jumlah tiket tidak valid atau melebihi stok.\n"; pauseClear(); return;
    }
    string kursiTerpilih[MAX_KURSI]; int jumlahDipilih = 0;
    pilihKursi(*f, kursiTerpilih, jumlahDipilih, jumlahTiket);
    int makananId[MAX_PESANAN_MAKANAN], makananJumlah[MAX_PESANAN_MAKANAN];
    int totalMakanan = 0, totalHarga = 0;
    prosesMakananDanRingkasan(*f, jadwal, kursiTerpilih, jumlahDipilih,
                              totalHarga, jumlahTiket, makananId, makananJumlah, totalMakanan);
    cout << "\nKonfirmasi pesan? (y/n): ";
    char konfirm; cin >> konfirm;
    if (konfirm == 'y' || konfirm == 'Y') {
        f->stok -= jumlahTiket; f->terjual += jumlahTiket;
        for (int i = 0; i < jumlahDipilih; i++)
            f->kursiTerisi[f->jumlahKursiTerisi++] = kursiTerpilih[i];
        saveFilms();
        prosesPembayaran(totalHarga);
        Pesanan* p = new Pesanan;
        p->username = loggedInUser->username;
        p->filmId = f->id;
        p->jadwal = jadwal;
        string kursiGabungan;
        for(int i  = 0; i < jumlahDipilih; i++)
            kursiGabungan += kursiTerpilih[i] + " ";
        p->kursiDipilih = kursiGabungan;
        for (int i = 0; i < totalMakanan; i++) {
            p->makananId[i] = makananId[i];
            p->makananJumlah[i] = makananJumlah[i];
        }
        p->totalMakanan = totalMakanan;
        p->totalHarga = totalHarga;
        p->next = nullptr;
        tambahPesanan(p);
        savePesanan();
        cout << "Pesanan berhasil!\n";
    } else cout << "Pesanan dibatalkan.\n";
    pauseClear();
}

void tampilRiwayatPesanan() {
    header("Riwayat Pesanan");
    cout << "Halo, " << loggedInUser->username << "\n\n";
    Pesanan* cur = pesananHead;
    int no = 1; bool ada = false;
    cout << "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n";
    cout << "|                  RIWAYAT PESANAN                  |\n";
    cout << "+=====+===============+===========+=================+\n";
    cout << "| NO  |    FILM       |  JADWAL   |     TOTAL       |\n";
    cout << "+=====+===============+===========+=================+\n";
    while (cur != NULL) {
        if (cur->username == loggedInUser->username) {
            ada = true;
            Film* f = cariFilmById(cur->filmId);
            string judul = "Unknown";
            if (f != NULL) {
                judul = f->judul;
                if (judul.length() > 13)
                    judul = judul.substr(0, 10) + "...";
            }
            cout << "| [" << setw(3) << no++ << "] | "
                 << setw(13) << left << judul << " | "
                 << setw(9) << left << cur->jadwal << " | Rp "
                 << setw(11) << left << cur->totalHarga << " |\n";
            cout << "+-----+---------------+-----------+-----------------+\n";
            cout << "|     | Kursi: ";
            istringstream iss(cur->kursiDipilih);
            string kursi; int kursiCount = 0;
            while (iss >> kursi) {
                if (kursiCount > 0 && kursiCount % 4 == 0) cout << "\n|     |        ";
                cout << "[" << kursi << "] ";
                kursiCount++;
            }
            cout << "\n|     | Makanan: ";
            if (cur->totalMakanan == 0) cout << "-";
            else for (int i = 0; i < cur->totalMakanan; ++i) {
                Makanan* m = cariMakananById(cur->makananId[i]);
                if (m != nullptr) {
                    if (i > 0) cout << ", ";
                    cout << m->nama << " [x" << cur->makananJumlah[i] << "]";
                }
            }
            cout << "\n+-----+---------------+-----------+-----------------+\n";
        }
        cur = cur->next;
    }
    if (!ada) {
        cout << "|                                                 |\n";
        cout << "|           BELUM ADA RIWAYAT PESANAN             |\n";
        cout << "|                                                 |\n";
        cout << "+=====+===============+===========+=================+\n";
    } else {
        cout << "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n";
    }
    pauseClear();
}

void adminLihatUser() {
    header("Daftar User");
    cout << "Halo, " << loggedInUser->username << "\n\n";
    cout << "Menu Admin\n";
    cout << "----------------------------------------\n";
    User* cur = userHead;
    cout << left << setw(20) << "Username" << setw(10) << "Admin\n";
    cout << "---------------------------\n";
    while (cur) {
        cout << setw(20) << cur->username << setw(10) << (cur->isAdmin ? "Ya" : "Tidak") << "\n";
        cur = cur->next;
    }
    pauseClear();
}

void adminTambahFilm() {
    header("Tambah Film");
    cout << "Halo, " << loggedInUser->username << "\n\n";
    cout << "Menu Admin\n";
    cout << "----------------------------------------\n";
    int idx = -1;
    for (int i=0; i<MAX_FILM; i++)
        if (films[i].id == 0) { idx = i; break; }
    if (idx == -1) { cout << "Data film sudah penuh.\n"; pauseClear(); return; }
    Film& f = films[idx];
    f.id = idx + 1;
    cout << "Judul film: "; cin.ignore(); getline(cin, f.judul);
    cout << "Stok kursi: "; cin >> f.stok;
    cout << "Durasi (menit): "; cin >> f.durasi;
    cout << "Jumlah jadwal: "; int jml; cin >> jml; f.jumlahJadwal = jml;
    for (int i = 0; i < MAX_JADWAL; i++) f.jadwal[i] = "";
    for (int i = 0; i < jml && i < MAX_JADWAL; i++) {
        cout << "Jadwal ke-" << (i+1) << ": "; cin >> f.jadwal[i];
    }
    f.terjual = 0; f.jumlahKursiTerisi = 0; f.jumlahUlasan = 0;
    for (int i = 0; i < MAX_KURSI; i++) f.kursiTerisi[i] = "";
    for (int i = 0; i < MAX_ULASAN; i++) { f.ulasan[i][0] = '\0'; f.rating[i] = 0; }
    saveFilms();
    cout << "Film berhasil ditambahkan.\n"; pauseClear();
}

void adminEditFilm() {
    header("Edit Film");
    cout << "Halo, " << loggedInUser->username << "\n\n";
    cout << "Menu Admin\n";
    cout << "----------------------------------------\n";
    tabelFilm();
    cout << "Masukkan ID film yang ingin diedit: "; int id; cin >> id;
    Film* f = cariFilmById(id);
    if (!f) { cout << "Film tidak ditemukan.\n"; pauseClear(); return; }
    cout << "Judul film (" << f->judul << "): "; cin.ignore(); string judulBaru; getline(cin, judulBaru);
    if (!judulBaru.empty()) f->judul = judulBaru;
    cout << "Stok kursi (" << f->stok << "): "; int stokBaru; cin >> stokBaru; if (stokBaru > 0) f->stok = stokBaru;
    cout << "Durasi (" << f->durasi << " menit): "; int durasiBaru; cin >> durasiBaru; if (durasiBaru > 0) f->durasi = durasiBaru;
    cout << "Ubah jadwal? (y/n): "; char pilih; cin >> pilih;
    if (pilih == 'y' || pilih == 'Y') {
        cout << "Jumlah jadwal: "; int jml; cin >> jml;
        for (int i = 0; i < MAX_JADWAL; i++) f->jadwal[i] = "";
        f->jumlahJadwal = 0;
        for (int i=0; i<jml; i++) {
            string j; cout << "Jadwal ke-" << (i+1) << ": "; cin >> j;
            if (f->jumlahJadwal < MAX_JADWAL) f->jadwal[f->jumlahJadwal++] = j;
        }
    }
    saveFilms();
    cout << "Film berhasil diedit.\n"; pauseClear();
}

void adminStatistik() {
    header("Statistik Penjualan");
    cout << "Halo, " << loggedInUser->username << "\n\n";
    cout << "Menu Admin\n";
    cout << "----------------------------------------\n";
    for (int i = 0; i < MAX_FILM - 1; i++)
        for (int j = 0; j < MAX_FILM - 1 - i; j++)
            if (films[j].terjual < films[j + 1].terjual) {
                Film temp = films[j];
                films[j] = films[j + 1];
                films[j + 1] = temp;
            }
    cout << "Film dengan penjualan terbanyak:\n";
    tabelFilm();
    pauseClear();
}

void adminBatalkanPesanan() {
    header("Batalkan Pesanan");
    cout << "Halo, " << loggedInUser->username << "\n\n";
    cout << "Menu Admin\n";
    cout << "----------------------------------------\n";
    Pesanan* cur = pesananHead;
    int no = 1; bool ada = false;
    cout << left << setw(5) << "No"
         << setw(20) << "Username"
         << setw(25) << "Film"
         << setw(15) << "Jadwal"
         << setw(15) << "Kursi"
         << "Total\n";
    cout << "-----------------------------------------------------------------------------------------\n";
    while (cur) {
        ada = true;
        Film* f = cariFilmById(cur->filmId);
        cout << setw(5) << no++;
        cout << setw(20) << cur->username;
        cout << setw(25) << (f ? f->judul : "Unknown");
        cout << setw(15) << cur->jadwal;
        string kursiStr = cur->kursiDipilih;
        if (!kursiStr.empty() && kursiStr.back() == ' ') kursiStr.pop_back();
        cout << setw(15) << kursiStr;
        cout << "Rp " << cur->totalHarga << "\n";
        cur = cur->next;
    }
    if (!ada) { cout << "Belum ada pesanan.\n"; pauseClear(); return; }
    cout << "Masukkan nomor pesanan yang ingin dibatalkan (0 untuk batal): ";
    int noPesanan; cin >> noPesanan;
    if (noPesanan == 0) return;
    cur = pesananHead; Pesanan* prev = NULL; int idx = 1;
    while (cur && idx < noPesanan) { prev = cur; cur = cur->next; idx++; }
    if (!cur) { cout << "Pesanan tidak ditemukan.\n"; pauseClear(); return; }
    Film* f = cariFilmById(cur->filmId);
    if (f) {
        int jumlahKursi = 0;
        istringstream iss(cur->kursiDipilih);
        string kursi;
        while (iss >> kursi) {
            jumlahKursi++;
            for (int j = 0; j < f->jumlahKursiTerisi; j++) {
                if (f->kursiTerisi[j] == kursi) {
                    for (int k = j; k < f->jumlahKursiTerisi - 1; k++) f->kursiTerisi[k] = f->kursiTerisi[k + 1];
                    f->kursiTerisi[f->jumlahKursiTerisi - 1] = "";
                    f->jumlahKursiTerisi--; break;
                }
            }
        }
        f->stok += jumlahKursi; f->terjual -= jumlahKursi;
        saveFilms();
    }
    if (prev) prev->next = cur->next; else pesananHead = cur->next;
    delete cur;
    savePesanan();
    cout << "Pesanan berhasil dibatalkan.\n";
    pauseClear();
}

void menuUser() {
    while (true) {
        header("Menu User");
        cout << "Halo, " << loggedInUser->username << "\n\n";
        cout << "Menu User\n";
        cout << "----------------------------------------\n";
        cout << "1. Lihat Daftar Film\n";
        cout << "2. Lihat Menu Makanan\n";
        cout << "3. Pesan Tiket\n";
        cout << "4. Riwayat Pesanan\n";
        cout << "5. Cari Film\n";
        cout << "6. Beri Rating/Ulasan Film\n";
        cout << "7. Logout\n";
        cout << "Pilihan: ";
        int pil; cin >> pil;
        switch (pil) {
            case 1:
                header("Daftar Film");
                cout << "Halo, " << loggedInUser->username << "\n\n";
                cout << "Menu User\n";
                cout << "----------------------------------------\n";
                tabelFilm();
                cout << "Tampilkan rating/ulasan film tertentu? (y/n): ";
                char lht; cin >> lht;
                if (lht == 'y' || lht == 'Y') {
                    cout << "Masukkan ID film: "; int id; cin >> id;
                    Film* f = cariFilmById(id);
                    if (f) tampilRatingUlasan(*f);
                }
                pauseClear();
                break;
            case 2:
                header("Menu Makanan");
                cout << "Halo, " << loggedInUser->username << "\n\n";
                cout << "Menu User\n";
                cout << "----------------------------------------\n";
                tabelMakanan();
                pauseClear();
                break;
            case 3: pesanTiket(); break;
            case 4: tampilRiwayatPesanan(); break;
            case 5: cariFilm(); break;
            case 6: isiRatingUlasan(); break;
            case 7: logout(); return;
            default: cout << "Pilihan tidak valid.\n"; pauseClear();
        }
    }
}

void menuAdmin() {
    while (true) {
        header("Menu Admin");
        cout << "Halo, " << loggedInUser->username << "\n\n";
        cout << "Menu Admin\n";
        cout << "----------------------------------------\n";
        cout << "1. Lihat Daftar Film\n";
        cout << "2. Tambah Film\n";
        cout << "3. Edit Film\n";
        cout << "4. Statistik Penjualan\n";
        cout << "5. Lihat User\n";
        cout << "6. Batalkan Pesanan\n";
        cout << "7. Cari Film\n";
        cout << "8. Logout\n";
        cout << "Pilihan: ";
        int pil; cin >> pil;
        switch (pil) {
            case 1:
                header("Daftar Film");
                cout << "Halo, " << loggedInUser->username << "\n\n";
                cout << "Menu Admin\n";
                cout << "----------------------------------------\n";
                tabelFilm();
                pauseClear();
                break;
            case 2: adminTambahFilm(); break;
            case 3: adminEditFilm(); break;
            case 4: adminStatistik(); break;
            case 5: adminLihatUser(); break;
            case 6: adminBatalkanPesanan(); break;
            case 7: cariFilm(); break;
            case 8: logout(); return;
            default: cout << "Pilihan tidak valid.\n"; pauseClear();
        }
    }
}

int main() {
    initData();
    loadUsers();
    loadFilms();
    loadPesanan();
    // Jika belum ada admin, buat default
    if (!userHead) {
        User* admin = new User;
        admin->username = "admin";
        admin->password = "admin123";
        admin->isAdmin = true;
        tambahUser(admin);
        saveUsers();
    }
    while (true) {
        header("Selamat Datang");
        cout << "1. Login\n";
        cout << "2. Daftar\n";
        cout << "3. Keluar\n";
        cout << "Pilih menu: ";
        int menu; cin >> menu;
        if (menu == 1) {
            if (login()) {
                if (loggedInUser->isAdmin) menuAdmin();
                else menuUser();
            }
        } else if (menu == 2) {
            daftar();
        } else if (menu == 3) {
            cout << "Terima kasih sudah menggunakan program ini.\n";
            break;
        } else {
            cout << "Pilihan tidak valid.\n";
            pauseClear();
        }
    }
    return 0;
}