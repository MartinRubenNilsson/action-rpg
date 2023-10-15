<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.10.2" name="naked_human" tilewidth="64" tileheight="64" tilecount="64" columns="8">
 <image source="mana_seed_human.png" width="512" height="512"/>
 <tile id="0" type="idle_d">
  <objectgroup draworder="index" id="2">
   <object id="7" x="24" y="32" width="16" height="16">
    <ellipse/>
   </object>
  </objectgroup>
 </tile>
 <tile id="8" type="idle_u"/>
 <tile id="16" type="idle_r"/>
 <tile id="24" type="idle_l"/>
 <tile id="32" type="walk_d">
  <animation>
   <frame tileid="32" duration="100"/>
   <frame tileid="33" duration="100"/>
   <frame tileid="34" duration="100"/>
   <frame tileid="35" duration="100"/>
   <frame tileid="36" duration="100"/>
   <frame tileid="37" duration="100"/>
  </animation>
 </tile>
 <tile id="35" type="run_d">
  <animation>
   <frame tileid="35" duration="80"/>
   <frame tileid="36" duration="55"/>
   <frame tileid="39" duration="125"/>
   <frame tileid="32" duration="80"/>
   <frame tileid="33" duration="55"/>
   <frame tileid="38" duration="125"/>
  </animation>
 </tile>
 <tile id="40" type="walk_u">
  <animation>
   <frame tileid="40" duration="100"/>
   <frame tileid="41" duration="100"/>
   <frame tileid="42" duration="100"/>
   <frame tileid="43" duration="100"/>
   <frame tileid="44" duration="100"/>
   <frame tileid="45" duration="100"/>
  </animation>
 </tile>
 <tile id="43" type="run_u">
  <animation>
   <frame tileid="43" duration="80"/>
   <frame tileid="44" duration="55"/>
   <frame tileid="47" duration="125"/>
   <frame tileid="40" duration="80"/>
   <frame tileid="41" duration="55"/>
   <frame tileid="46" duration="125"/>
  </animation>
 </tile>
 <tile id="48" type="walk_r">
  <animation>
   <frame tileid="48" duration="100"/>
   <frame tileid="49" duration="100"/>
   <frame tileid="50" duration="100"/>
   <frame tileid="51" duration="100"/>
   <frame tileid="52" duration="100"/>
   <frame tileid="53" duration="100"/>
  </animation>
 </tile>
 <tile id="51" type="run_r">
  <animation>
   <frame tileid="51" duration="80"/>
   <frame tileid="52" duration="55"/>
   <frame tileid="55" duration="125"/>
   <frame tileid="48" duration="80"/>
   <frame tileid="49" duration="55"/>
   <frame tileid="54" duration="125"/>
  </animation>
 </tile>
 <tile id="56" type="walk_l">
  <animation>
   <frame tileid="56" duration="100"/>
   <frame tileid="57" duration="100"/>
   <frame tileid="58" duration="100"/>
   <frame tileid="59" duration="100"/>
   <frame tileid="60" duration="100"/>
   <frame tileid="61" duration="100"/>
  </animation>
 </tile>
 <tile id="59" type="run_l">
  <animation>
   <frame tileid="59" duration="80"/>
   <frame tileid="60" duration="55"/>
   <frame tileid="63" duration="125"/>
   <frame tileid="56" duration="80"/>
   <frame tileid="57" duration="55"/>
   <frame tileid="62" duration="125"/>
  </animation>
 </tile>
</tileset>
