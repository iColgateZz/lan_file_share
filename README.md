# LAN File Share

Share files with everyone on the local area network!  
Spawn an HTTP server in a directory and let everyone access its contents via a browser!  
Customize the UI by modifying the frontend code!

Basically, this is an extended version of `python3 -m http.server`.

## Compilation

Run `make` on *nix machines. This command produces the `share` executable file that can be launched.

## Usage

After compilation, you can launch the program by running `./share localhost 8080`. Notice that if you open any browser of your choice and navigate to `http://127.0.0.1:8080`, you will see the contents of the directory. Click on the file names to open or download them, and click on directories to navigate into them. Notice that clicking `..` brings you one directory up the filesystem tree.

To use the program in any directory of your choice, first modify the `PATH_TO_TEMPLATE_DIR` macro to be the full path to the `static` directory on your machine, e.g., `/home/user/lan_file_share/static`. Then, recompile the executable using `make`. Finally, place the executable into your PATH. If you want the executable to be in the `/usr/local/bin` directory, you can run `sudo mv share /usr/local/bin`.

Now, go to any directory of your choice and run `share npa 8080`. Here, `npa` stands for "no particular address," which evaluates to `0.0.0.0`, meaning that the HTTP server will be accessible on all local area networks your machine is connected to. If you want to specify a certain LAN, run `share <ip> <port>`, where `<ip>` is the address of your machine in that particular LAN.

Others can access the files by navigating to `http://<ip>:<port>`.

## Customization

The `static` directory also contains a `.js` and a `.css` file. While they do not significantly enhance the UI, they serve as examples of how to add more files and import them into the HTML template to improve the frontend.

Let's take a closer look at the `static/template.html` file. There are some special placeholders there. `#TITLE` will be replaced with `LISTING of {path}`, and `#LISTING` will be replaced with the links to different files and directories. `PATH_TO_TEMPLATE_DIR` is a special value used to hide the full path to the `static` directory, which may contain sensitive information that you might not want to share.
