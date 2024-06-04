# hagelslag

cool scanner <(= w =)>

## Why

I wanted to practice some C and I've heard of scanners for Minecraft servers some time ago and decided to do something similar. I'm not following any guides or searching on how scanners (in general) work. Only Linux is supported.

My idea with this project is to try and add as many features as I can just so I can try things out in C, this is just an educational project for me.

As I am not a C programmer, Im sure I am either not doing something the C way, or just being plain stupid. I am not following any standards, just solving my own problems, one of them being 'how the hell do I store 3.7 billion integers properly'.

The `hagelslag` name came from a friend, well, eating a [Hagelslag](https://en.wikipedia.org/wiki/Hagelslag).

## How

`hagelslag` works by generating all possible IP addresses, all 4.3 billion of them, in a loop, checking if they are [reserved](https://en.wikipedia.org/wiki/Reserved_IP_addresses) then sending them to a `ThreadPool` for scanning.

Each thread will retrieve an amount of `Task`s from the `Queue` inside the thread pool, removing them from the queue, and process these tasks.

A thread will block and for each task inside of it, try to `connect` to the address. If the connection succeeds it will then try to send a `GET` to that address. If a `GET` is successful the address will be added to the `http` table in the sqlite database.

The IP address will be converted to an integer before being added to the database.

## Ideas

- As of now, `hagelslag` will take ages to finish scanning just once, use non-blocking sockets or anything thats lets me fire and forget sockets spawning/working.

- Adding to the above, 'how the hell do I stop my computer bursting into flames when it starts spamming database inserts and thousands of sockets being created/used/destroyed'.

- Inserting into the database after every task finishes processing sucks, use transactions.

- Add a Minecraft scanner as a second scanner.

- Allow for configuration, I want to try not using a config file.

- More error handling.

- Spread logging properly, right now its either too spammy or too little information being logged on some levels.

- Maybe improve the http scanner to only add IPs that return a html page.

- Remove this weird virus that keeps adding Frieren in the code.
