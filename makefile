nome_eseguibile = rfid
oggetti = gesttransponder.o global.o main.o mfrc522.o mfrc630.o post_http.o

# Flag di compilazione: -g aggiunge i simboli di debug, -O0 disabilita le ottimizzazioni che potrebbero confondere GDB durante il tracciamento delle righe di codice
CXXFLAGS = -g -O0

$(nome_eseguibile): $(oggetti)
	g++ $(CXXFLAGS) $(oggetti) -lwiringPi -lpthread -o $(nome_eseguibile)

gesttransponder.o: gesttransponder.cpp
	g++ $(CXXFLAGS) -c gesttransponder.cpp

global.o: global.cpp
	g++ $(CXXFLAGS) -c global.cpp

main.o: main.cpp
	g++ $(CXXFLAGS) -c main.cpp

mfrc522.o: mfrc522.cpp
	g++ $(CXXFLAGS) -c mfrc522.cpp

mfrc630.o: mfrc630.cpp
	g++ $(CXXFLAGS) -c mfrc630.cpp

post_http.o: post_http.cpp
	g++ $(CXXFLAGS) -c post_http.cpp

clean:
	rm -f $(oggetti) $(nome_esegui
