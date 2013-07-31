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
#level-type=FLAT
#generator-settings=2;7,3x1,52x42;2
#generate-structures=false
#gamemode=1
EOF
${EDITOR:-vi} server.properties
java -jar $JAR nogui
cd ..
rm -rf $TMP
