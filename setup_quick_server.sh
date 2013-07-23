#!/bin/bash
TMP=`mktemp -d -p .`
cd $TMP
URL="https://s3.amazonaws.com/Minecraft.Download/versions/1.6.2/minecraft_server.1.6.2.jar"
JAR="minecraft_server.jar"
wget $URL -O $JAR
cat > server.properties << EOF
server-ip=127.0.0.1
server-port=25565
motd=A Minecraft Server
EOF
${EDITOR:-vi} server.properties
java -jar $JAR nogui