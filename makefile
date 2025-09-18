nome_eseguibile = rfid
oggetti = gesttransponder.o global.o main.o mfrc522.o mfrc630.o post_http.o 

$(nome_eseguibile): $(oggetti)
	g++ $(oggetti) -lwiringPi -lpthread -o $(nome_eseguibile)

gesttransponder.o: gesttransponder.cpp
	g++ -c gesttransponder.cpp

global.o: global.cpp
	g++ -c global.cpp

main.o: main.cpp
	g++ -c main.cpp

mfrc522.o: mfrc522.cpp
	g++ -c mfrc522.cpp

mfrc630.o: mfrc630.cpp
	g++ -c mfrc630.cpp

post_http.o: post_http.cpp
	g++ -c post_http.cpp

clean:
	rm -f $(oggetti) $(nome_eseguibile)
