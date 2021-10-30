# socket-programming-c
Implementation of Socket in C | Part of Operating System (A) class of ITS

* ### a - Register, Login, >1 Client
  #### Deskripsi Soal
  * Server dapat menerima lebih dari 1 koneksi Client sekaligus, namun hanya 1 Client yang dapat menggunakan fungsi-fungsi program. Client ke-2 dan seterusnya harus menunggu sampai Client pertama log out
  * Client diminta `id` dan `password` saat register -> disimpan di file `akun.txt` sebagai
    <pre>
    id:password
    id2:password2
    </pre>
  * Client diminta `id` dan `password` saat login -> dicocokkan dengan `id:password` yang ada di file `akun.txt`
  
  #### Solusi Soal
  * Konesi Client-Server
    * gunakan template koneksi socket Client dan Server yang ada di modul 3
    * manfaatkan thread untuk dapat memungkinkan terjadinya >1 koneksi Client ke Server
      ```c
      while(true) {
        if ((new_socket = accept(server_fd, 
            (struct sockaddr *) &address, (socklen_t*) &addr_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pthread_create(&(tid[total]), NULL, &client, &new_socket);
        total++;
      }
      ```
    * simpan jumlah Client yang sedang terkoneksi pada suatu variabel
    * tentukan apakah Client akan di terima atau di tahan aksesnya ke fungsi program berdasarkan jumlah koneksi
      ```c
      if (total == 1) {
        send(new_socket, hello, STR_SIZE, 0);
      }
      else {
          send(new_socket, deny, STR_SIZE, 0);
      }
      ```
    * di sisi Client, akan diperiksa pesan yang diterima dari Server
      ```c
      while (!equal(hello, buffer))
      ```
      jika Server tidak mengirim pesan yang menandakan Client dapat menggunakan program, tunggu di `while` loop
    * kembali di sisi Server, selain meng-handle Client pertama, buat Client tersebut untuk menunggu
      ```c
      while (total > 1) {
        valread = read(new_socket, buffer, STR_SIZE);
        if (total == 1) {
            send(new_socket, hello, STR_SIZE, 0);
        }
        else {
            send(new_socket, deny, STR_SIZE, 0);
        }
      }
      ```
      ![image](https://user-images.githubusercontent.com/43901559/118665759-c7771f80-b81c-11eb-878f-450c16789cba.png)
      >tampilan di sisi Client
    * ketika meng-handle Client pertama, siapkan Server untuk menunggu command dari Client. Seperti: login, register, dst.
      
      ![image](https://user-images.githubusercontent.com/43901559/118665671-ae6e6e80-b81c-11eb-84aa-8a34d7bd037a.png)
      >tampilan di sisi Client
    * ketika Client memutus koneksinya, decrement jumlah koneksi
      ```c
      else if (equal(quit, buffer)) {
        close(new_socket);
        total--;
        break;
      }
      ```
    * terima Client yang sedang menunggu apabila menjadi koneksi pertama
      
      ![image](https://user-images.githubusercontent.com/43901559/118665952-f7262780-b81c-11eb-8d51-afc44d12f646.png)
      >tampilan di sisi Client
  * Fungsi register
    * Client mengirimkan command `register` ke Server
    * Server menerima command dan menyiapkan file `akun.txt`
      ```c
      fp2 = fopen("akun.txt", "a");
      ```
    * Client-Server kirim-terima username dan password yang ingin di-regist
      
      ![image](https://user-images.githubusercontent.com/43901559/118666528-761b6000-b81d-11eb-9c2e-ff0565cb28cc.png)
      >tampilan di Client
    * `fprintf`-kan user creds yang baru ke file `akun.txt`
    * `close` file dan akun telah diregister
  * Fungsi login
    * Client mengirimkan command `login` ke Server
    * Server menerima command
    * Client-Server kirim-terima username dan password
      
      ![image](https://user-images.githubusercontent.com/43901559/118667260-0ce81c80-b81e-11eb-9395-485faa5c2610.png)
      >tampilan di Client
    * Server membuka file `akun.txt`
    * Baca file line-by-line dan cari login credentials yang sesuai
      ```c
      while ((file_read = getline(&line, &len, fp3) != -1))
      ```
    * Kirim pesan ke Client apakah login berhasil atau tidak
    * Jika login berhasil, siapkan Client-Server untuk command add, download, dst.
      
      ![image](https://user-images.githubusercontent.com/43901559/118667562-520c4e80-b81e-11eb-95a5-5c7980442145.png)
      >tampilan di Client
  #### Kendala Soal
  * sulit untuk menerapkan fungsionalitas Client yang terkoneksi >1 dan ada Client yang harus menunggu
  
* ### b - files.txt dan folder FILES
  #### Deskripsi Soal
  * di Server terdapat `files.tsv` yang menyimpan
    <pre>
    path file di server
    publisher
    tahunpublikasi
    </pre>
    `files.tsv` di-update setiap ada operasi add dan delete files
  * folder `FILES` yang menyimpan semua file yang dikirimkan oleh Client, otomatis dibuat saat server dijalankan
  
  #### Solusi Soal
  * `files.tsv`
    * pada operasi penambahan file, tambahkan kode yang akan membuat file `files.tsv`
    * perubahan pada file tersebut akan dijelaskan lebih lanjut pada bagian soal selanjutnya
  * folder `FILES`
    * buat folder `FILES` setelah Server dijalankan
  
  #### Kendala Soal
  * tidak ada
  
* ### c - uploading files
  #### Deskripsi Soal
  * file yang dikirim ke Server disimpan di folder `FILES`
    <pre>
    File1.ekstensi
    File2.ekstensi
    </pre>
  * client mengirimkan command `add` dan Client memasukkan detail file
    <pre>
    Publisher:
    Tahun Publikasi:
    Filepath:
    </pre>
  * `files.tsv` menyimpan data file yang dikirimkan ke Server
  
  #### Solusi Soal
  * command `add` dari Client
    * Client menerima command dan meneruskannya ke Server
    * Server bersiap untuk menerima detail dari file
    * Client mengirimkan detail dari file
    
      ![image](https://user-images.githubusercontent.com/43901559/118669933-5df91000-b820-11eb-9ba3-0f5677b741db.png)
      >tampilan di Client
  * pengiriman file ke Server
    * Client membuka file dan mengirimkan file ke Server
      ```c
      int fd = open(data, O_RDONLY);
      if (!fd) {
        perror("can't open");
        exit(EXIT_FAILURE);
      }
      
      int read_len;
      while (true) {
        memset(data, 0x00, STR_SIZE);
        read_len = read(fd, data, STR_SIZE);

        if (read_len == 0) {
          break;
        }
        else {
          send(sock, data, read_len, 0);                               
        }
      }
      close(fd);
      ```
    * Server menerima file dan menyimpannya
      ```c
      int des_fd = open(request.path, O_WRONLY | O_CREAT | O_EXCL, 0700);
      if (!des_fd) {
        perror("can't open file");
        exit(EXIT_FAILURE);
      }
      ```
      >Server bersiap untuk menyimpan file
      
      ![image](https://user-images.githubusercontent.com/43901559/118670586-f1cadc00-b820-11eb-88e5-7b934510e285.png)
      >struktur folder di Server
  * penambahan detail file di `files.tsv`
    * Server menerima detail file terlebih dahulu
      ```c
      valread = read(new_socket, request.publisher, STR_SIZE);
      valread = read(new_socket, request.year, STR_SIZE);
      valread = read(new_socket, clientPath, STR_SIZE);
      ```
    * Server memproses file path dari Client dan memodifikasinya agar dapat menyimpan file di `FILES/nama_file.extension`
    * Server membuka `files.tsv` dan menambahkan detail file
      ```c
      fp = fopen("files.tsv", "a");
      fprintf(fp, "%s\t%s\t%s\n", request.path, request.publisher, request.year);
      fclose(fp);
      ```
      ![image](https://user-images.githubusercontent.com/43901559/118671543-c3013580-b821-11eb-9743-fc1cb1e21e5e.png)
      >isi `files.tsv`
  
  #### Kendala Soal
  * kesulitan mengerjakan soal

  #### Referensi
  * https://stackoverflow.com/questions/40786888/send-file-from-client-to-server-using-socket-in-c-on-linux
  * https://stackoverflow.com/questions/3501338/c-read-file-line-by-line

* ### d - downloading files
  #### Deskripsi Soal
  * Client dapat mendownload file yang ada di Server -> check di `files.tsv`
  
  #### Solusi Soal
  * download file
    * Solusi dari download sangat serupa dengan solusi dari add, hanya saja peran Client-Server dibalik
    * Client mengirimkan command `download nama_file.extension`
    * Server menerima command dan mencari nama file di `files.tsv`
      
      ![image](https://user-images.githubusercontent.com/43901559/118672984-d82a9400-b822-11eb-9a07-3d96c6f27405.png)
      >jika file bisa di-download

      ![image](https://user-images.githubusercontent.com/43901559/118673023-e1b3fc00-b822-11eb-9532-a533eabd6fd6.png)
      >file tidak ada di Server
    * Server akan mengirimkan file ke Client jika ada file yang diminta
  
  #### Kendala Soal
  * kendala serupa dengan kendala di 1.c :)
  
* ### e - deleting files
  #### Deskripsi Soal
  * Client mengirimkan command `delete nama_file.extension`
  * Jika file ada, file hanya di-rename menjadi `old-nama_file.extension`
  * Hapus file dari list pada `files.tsv`
  
  #### Solusi Soal
  * Client send command
    * Client mengirimkan command delete dan nama file yang ingin di-delete
    * Server menerima command dan mengecek apakah ada file tersebut di `files.tsv`
    
      ![image](https://user-images.githubusercontent.com/43901559/118673949-9f3eef00-b823-11eb-97ea-f3af9e775029.png)
      >tampilan di Client apabila file ada dan telah terhapus

      ![image](https://user-images.githubusercontent.com/43901559/118674062-b4b41900-b823-11eb-82e3-18a58805898f.png)
      >tampilan di Client apabila file tidak ada

      ![image](https://user-images.githubusercontent.com/43901559/118674123-c09fdb00-b823-11eb-975b-ec40da756f00.png)
      >tampilan di Client apabila Client ingin menghapus file yang telah dihapus
  * Rename file
    * jika ada file yang ingin dihapus, rename nama file
      ```c
      found = true;
      char old[] = "FILES/old-";
      strcat(old, temp_entry.name);
      rename(temp_entry.path, old);
      ```
  * Ubah `files.tsv`
    * selama membaca `files.tsv`, simpan pada baris ke-berapa terdapat detail dari file yang dihapus
    * buat fungsi untuk menghapus baris tertentu di `files.tsv`
      ```c
      removeLine(index);
      ```
      >panggil fungsi
    * fungsi merupakan modifikasi dari fungsi yang ditemukan di-web. Link tertera  
  
  #### Kendala Soal
  * tidak ada
  
  #### Referensi
  * fungsi remove line di file https://www.w3resource.com/c-programming-exercises/file-handling/c-file-handling-exercise-8.php
  
* ### f - get all files details
  #### Deskripsi Soal
  * Client mengirimkan command `see`
  * Format output di Client sebagai berikut:
    <pre>
    Nama:
    Publisher:
    Tahun publishing:
    Ekstensi File :
    Filepath :
    
    Nama:
    Publisher:
    Tahun publishing:
    Ekstensi File :
    Filepath :
    </pre>
  
  #### Solusi Soal
  * Client mengirimkan command `see`
    * Client-Server kirim-terima command
    * Server membuka `files.tsv`
  * Output di client
    * Server membaca `files.tsv` line-by-line
    * Server memproses tiap line agar mendapatkan informasi yang diperlukan
    * Selama masih ada baris untuk diproses, kirim informasi ke Client
      ```c
      char message[STR_SIZE];
      sprintf(message, "Nama : %s\nPublisher : %s\nTahun Publishing : %s\nEkstensi File : %s\nFilepath : %s\n\n", 
        temp_entry.name, temp_entry.publisher, temp_entry.year, ext, temp_entry.path);
                                
      send(new_socket, message, STR_SIZE, 0);
      ```
    * Client menerima informasi dari Server selama masih ada informasi untuk diterima
      
      ![image](https://user-images.githubusercontent.com/43901559/118676708-d31b1400-b825-11eb-96c6-1492ebfd5316.png)
      >tampilan di Client
  
  #### Kendala Soal
  * Kesulitan bagaimana client dapat menerima file selama masih ada file untuk diterima dan berhenti ketika tidak ada informasi untuk diterima
  
* ### g - WHERE LIKE
  #### Deskripsi Soal
  * Client menerima detail files yang mengandung string yang dikirimkan
  * Format output sama dengan 1.f
  
  #### Solusi Soal
  * Client send command
    * Client-Server kirim-terima
    * Server membaca `files.tsv` line-by-line
    * Manfaatkan strstr agar sesuai dengan apa yang diminta soal
      ```c
      if ((h = strstr(temp_entry.name, buffer)) != NULL)
      ```
  * Format output sama dengan 1.f
    * lihat penjelasan di 1.f 
  
  #### Kendala Soal
  * tidak paham cara pakai strstr

  #### Referensi
  * https://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
  
* ### h - log additions and deletions
  #### Deskripsi Soal
  * tiap operasi add dan delete, log di `running.log`
  * format `running.log`
    <pre>
    Tambah : File1.ektensi (id:pass)
    Hapus : File2.ektensi (id:pass)
    </pre>
  
  #### Solusi Soal
  * log tiap add dan delete
    * buat fungsi untuk memudahkan penambahan log di `running.log`
      ```c
      void log_action(char *type, char *fileName, char *user, char *pass) {
          FILE *log;
          char action[16];

          if (equal(type, "add")) {
              strcpy(action, "Tambah");
          }
          else if (equal(type, "delete")) {
              strcpy(action, "Hapus");
          }

          log = fopen("running.log", "a");
          fprintf(log, "%s : %s (%s:%s)\n", action, fileName, user, pass);
          fclose(log);

          return;
      }
      ```
      >fungsi menerima apa tipe operasinya (add/delete), file apa yang terpengaruh, siapa yang melakukannya beserta passwordnya
    * ketika Server memproses add dan delete, panggil fungsi tersebut
      ```c
      log_action("add", request.name, akun.name, akun.password);
      ```
      >panggil fungsi ketika terjadi file addition
      ```c
      log_action("delete", temp_entry.name, akun.name, akun.password);
      ```
      >panggil fungsi ketika terjadi deletion
  * format `running.log`
    * isi `running.log`
      
      ![image](https://user-images.githubusercontent.com/43901559/118678848-92bc9580-b827-11eb-8ea6-18bf6024d7c0.png)
      
  #### Kendala Soal
    * tidak ada

* ### appendix
  demi pemudahan pengerjaan soal, dibuat beberapa `struct`
  #### `Account`
    ```c
    typedef struct akun_t {
      char name[SIZE];
      char password[SIZE];
    } Account;
    ```
    >memudahkan login-register dan juga ketika log di 1.h
  
  #### `Entry`
    ```c
    typedef struct file_t {
      char publisher[SIZE];
      char year[SIZE];
      char name[SIZE];
      char path[SIZE];
    } Entry;
    ```
    >memudahkan semua operasi yang berhubungan dengan file terutama ketika string processing

