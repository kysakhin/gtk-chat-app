# GTK chat application
This is a very simple GTK app in C, just for the sake of learning, made a simple chat app that runs on the local machine.


if you want to test this, clone the repo
```bash
git clone https://github.com/kysakhin/gtk-chat-app.git 
cd gtk-chat-app
```

make sure you clean the existing binary before making one.
```bash
make clean

make
```

run the executable
```bash
./chat_app
```

and then start another instance of the same, and then you can click on the "Start accepting" button on the server.




### key takeaways
a problem that ive faced was that the accept function, blocks on the main thread which makes the ui not appear at all. so made a button to start initilize listening on the port. 
