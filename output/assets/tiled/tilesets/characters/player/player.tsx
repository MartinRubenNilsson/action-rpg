<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.10.2" name="player" tilewidth="64" tileheight="64" tilecount="256" columns="16">
 <image source="fbas_01body_human_00.png" width="1024" height="1024"/>
 <tile id="0" type="idle_d">
  <objectgroup draworder="index" id="2">
   <object id="1" x="24" y="32" width="16" height="16">
    <ellipse/>
   </object>
   <object id="2" name="pivot" x="32" y="42">
    <point/>
   </object>
  </objectgroup>
 </tile>
 <tile id="16" type="idle_u"/>
 <tile id="32" type="idle_r"/>
 <tile id="48" type="walk_d">
  <animation>
   <frame tileid="48" duration="135"/>
   <frame tileid="49" duration="135"/>
   <frame tileid="50" duration="135"/>
  </animation>
 </tile>
 <tile id="51" type="run_d">
  <animation>
   <frame tileid="48" duration="80"/>
   <frame tileid="49" duration="55"/>
   <frame tileid="51" duration="155"/>
  </animation>
 </tile>
 <tile id="52" type="walk_u">
  <animation>
   <frame tileid="52" duration="135"/>
   <frame tileid="53" duration="135"/>
   <frame tileid="54" duration="135"/>
  </animation>
 </tile>
 <tile id="55" type="run_u">
  <animation>
   <frame tileid="52" duration="80"/>
   <frame tileid="53" duration="55"/>
   <frame tileid="55" duration="155"/>
  </animation>
 </tile>
 <tile id="64" type="walk_r">
  <animation>
   <frame tileid="64" duration="135"/>
   <frame tileid="65" duration="135"/>
   <frame tileid="66" duration="135"/>
   <frame tileid="67" duration="135"/>
   <frame tileid="68" duration="135"/>
   <frame tileid="69" duration="135"/>
  </animation>
 </tile>
 <tile id="70" type="run_r">
  <animation>
   <frame tileid="64" duration="80"/>
   <frame tileid="65" duration="55"/>
   <frame tileid="70" duration="155"/>
   <frame tileid="67" duration="80"/>
   <frame tileid="68" duration="55"/>
   <frame tileid="71" duration="155"/>
  </animation>
 </tile>
</tileset>
