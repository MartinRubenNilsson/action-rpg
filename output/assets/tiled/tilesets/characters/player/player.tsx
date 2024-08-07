<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.10.2" name="player" tilewidth="64" tileheight="64" tilecount="256" columns="16">
 <image source="../../../../textures/character/01body/fbas_01body_human_00.png" width="1024" height="1024"/>
 <tile id="0" type="idle_d"/>
 <tile id="16" type="idle_u"/>
 <tile id="32" type="idle_r"/>
 <tile id="48" type="walk_d">
  <properties>
   <property name="step" value=""/>
  </properties>
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
  <properties>
   <property name="step" value=""/>
  </properties>
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
  <properties>
   <property name="step" value=""/>
  </properties>
  <animation>
   <frame tileid="64" duration="135"/>
   <frame tileid="65" duration="135"/>
   <frame tileid="66" duration="135"/>
   <frame tileid="67" duration="135"/>
   <frame tileid="68" duration="135"/>
   <frame tileid="69" duration="135"/>
  </animation>
 </tile>
 <tile id="67">
  <properties>
   <property name="step" value=""/>
  </properties>
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
 <tile id="132" type="sword_attack_d">
  <animation>
   <frame tileid="132" duration="180"/>
   <frame tileid="133" duration="80"/>
   <frame tileid="12" duration="380"/>
  </animation>
 </tile>
 <tile id="133" type="bow_shot_d">
  <properties>
   <property name="strike" value=""/>
  </properties>
  <animation>
   <frame tileid="133" duration="160"/>
   <frame tileid="134" duration="500"/>
   <frame tileid="135" duration="460"/>
  </animation>
 </tile>
 <tile id="135">
  <properties>
   <property name="shoot" value=""/>
  </properties>
 </tile>
 <tile id="148" type="sword_attack_u">
  <animation>
   <frame tileid="148" duration="180"/>
   <frame tileid="149" duration="80"/>
   <frame tileid="28" duration="380"/>
  </animation>
 </tile>
 <tile id="149" type="bow_shot_u">
  <properties>
   <property name="strike" value=""/>
  </properties>
  <animation>
   <frame tileid="149" duration="160"/>
   <frame tileid="150" duration="500"/>
   <frame tileid="151" duration="460"/>
  </animation>
 </tile>
 <tile id="151">
  <properties>
   <property name="shoot" value=""/>
  </properties>
 </tile>
 <tile id="164" type="sword_attack_r">
  <animation>
   <frame tileid="164" duration="180"/>
   <frame tileid="165" duration="80"/>
   <frame tileid="44" duration="380"/>
  </animation>
 </tile>
 <tile id="165" type="bow_shot_r">
  <properties>
   <property name="strike" value=""/>
  </properties>
  <animation>
   <frame tileid="165" duration="160"/>
   <frame tileid="166" duration="500"/>
   <frame tileid="167" duration="460"/>
  </animation>
 </tile>
 <tile id="167">
  <properties>
   <property name="shoot" value=""/>
  </properties>
 </tile>
 <tile id="178" type="dying_r">
  <animation>
   <frame tileid="178" duration="200"/>
   <frame tileid="179" duration="200"/>
   <frame tileid="180" duration="100"/>
  </animation>
 </tile>
 <tile id="180" type="dead_l"/>
 <tile id="181" type="dying_u">
  <animation>
   <frame tileid="181" duration="200"/>
   <frame tileid="182" duration="200"/>
   <frame tileid="183" duration="100"/>
  </animation>
 </tile>
 <tile id="183" type="dead_u"/>
</tileset>
