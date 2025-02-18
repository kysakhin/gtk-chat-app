# GTK chat application
This is a very simple GTK app in C, just for the sake of learning, made a simple chat app that runs on the local machine.

# Dependencies

- gcc
- make
- gtk4 development libraries
if you're on arch, 
```bash
sudo pacman -S gtk4
```

if you're on a debian based system
```bash
sudo apt install libgtk-4-dev
```

- pgk-config: helps gcc locate the correct gtk4 headers and libraries
if you're on arch
```bash
sudo pacman -S pkgconf
```

debian based
```bash
sudo apt install pkg-config
```

# Running

clone the repo
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
