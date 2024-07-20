<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.10.0" name="eternal dungeon" tilewidth="16" tileheight="16" tilecount="256" columns="16">
 <image source="../eternal dungeon.png" width="256" height="256"/>
 <tile id="5">
  <animation>
   <frame tileid="5" duration="240"/>
   <frame tileid="6" duration="240"/>
   <frame tileid="7" duration="240"/>
   <frame tileid="8" duration="240"/>
  </animation>
 </tile>
 <tile id="21">
  <animation>
   <frame tileid="21" duration="240"/>
   <frame tileid="22" duration="240"/>
   <frame tileid="23" duration="240"/>
   <frame tileid="24" duration="240"/>
  </animation>
 </tile>
 <wangsets>
  <wangset name="dungeon wall tops" type="corner" tile="-1">
   <wangcolor name="blue top" color="#0055ff" tile="-1" probability="1"/>
   <wangcolor name="green top" color="#00ff00" tile="-1" probability="1"/>
   <wangcolor name="red top" color="#ff0000" tile="-1" probability="1"/>
   <wangtile tileid="144" wangid="0,0,0,1,0,0,0,0"/>
   <wangtile tileid="145" wangid="0,0,0,1,0,1,0,0"/>
   <wangtile tileid="146" wangid="0,0,0,0,0,1,0,0"/>
   <wangtile tileid="147" wangid="0,1,0,0,0,1,0,1"/>
   <wangtile tileid="148" wangid="0,1,0,1,0,0,0,1"/>
   <wangtile tileid="149" wangid="0,0,0,2,0,0,0,0"/>
   <wangtile tileid="150" wangid="0,0,0,2,0,2,0,0"/>
   <wangtile tileid="151" wangid="0,0,0,0,0,2,0,0"/>
   <wangtile tileid="152" wangid="0,2,0,0,0,2,0,2"/>
   <wangtile tileid="153" wangid="0,2,0,2,0,0,0,2"/>
   <wangtile tileid="154" wangid="0,0,0,3,0,0,0,0"/>
   <wangtile tileid="155" wangid="0,0,0,3,0,3,0,0"/>
   <wangtile tileid="156" wangid="0,0,0,0,0,3,0,0"/>
   <wangtile tileid="157" wangid="0,3,0,0,0,3,0,3"/>
   <wangtile tileid="158" wangid="0,3,0,3,0,0,0,3"/>
   <wangtile tileid="160" wangid="0,1,0,1,0,0,0,0"/>
   <wangtile tileid="161" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="162" wangid="0,0,0,0,0,1,0,1"/>
   <wangtile tileid="163" wangid="0,0,0,1,0,1,0,1"/>
   <wangtile tileid="164" wangid="0,1,0,1,0,1,0,0"/>
   <wangtile tileid="165" wangid="0,2,0,2,0,0,0,0"/>
   <wangtile tileid="166" wangid="0,2,0,2,0,2,0,2"/>
   <wangtile tileid="167" wangid="0,0,0,0,0,2,0,2"/>
   <wangtile tileid="168" wangid="0,0,0,2,0,2,0,2"/>
   <wangtile tileid="169" wangid="0,2,0,2,0,2,0,0"/>
   <wangtile tileid="170" wangid="0,3,0,3,0,0,0,0"/>
   <wangtile tileid="171" wangid="0,3,0,3,0,3,0,3"/>
   <wangtile tileid="172" wangid="0,0,0,0,0,3,0,3"/>
   <wangtile tileid="173" wangid="0,0,0,3,0,3,0,3"/>
   <wangtile tileid="174" wangid="0,3,0,3,0,3,0,0"/>
   <wangtile tileid="176" wangid="0,1,0,0,0,0,0,0"/>
   <wangtile tileid="177" wangid="0,1,0,0,0,0,0,1"/>
   <wangtile tileid="178" wangid="0,0,0,0,0,0,0,1"/>
   <wangtile tileid="181" wangid="0,2,0,0,0,0,0,0"/>
   <wangtile tileid="182" wangid="0,2,0,0,0,0,0,2"/>
   <wangtile tileid="183" wangid="0,0,0,0,0,0,0,2"/>
   <wangtile tileid="186" wangid="0,3,0,0,0,0,0,0"/>
   <wangtile tileid="187" wangid="0,3,0,0,0,0,0,3"/>
   <wangtile tileid="188" wangid="0,0,0,0,0,0,0,3"/>
   <wangtile tileid="243" wangid="0,1,0,0,0,1,0,0"/>
   <wangtile tileid="244" wangid="0,0,0,1,0,0,0,1"/>
   <wangtile tileid="248" wangid="0,2,0,0,0,2,0,0"/>
   <wangtile tileid="249" wangid="0,0,0,2,0,0,0,2"/>
   <wangtile tileid="253" wangid="0,3,0,0,0,3,0,0"/>
   <wangtile tileid="254" wangid="0,0,0,3,0,0,0,3"/>
  </wangset>
  <wangset name="dungeon floors" type="corner" tile="-1">
   <wangcolor name="diamond tile" color="#ff557f" tile="-1" probability="1"/>
   <wangcolor name="brick floor" color="#00ffff" tile="-1" probability="1"/>
   <wangtile tileid="102" wangid="0,0,0,1,0,0,0,0"/>
   <wangtile tileid="103" wangid="0,0,0,1,0,1,0,0"/>
   <wangtile tileid="104" wangid="0,0,0,0,0,1,0,0"/>
   <wangtile tileid="105" wangid="0,1,0,0,0,1,0,1"/>
   <wangtile tileid="106" wangid="0,1,0,1,0,0,0,1"/>
   <wangtile tileid="107" wangid="0,0,0,2,0,0,0,0"/>
   <wangtile tileid="108" wangid="0,0,0,2,0,2,0,0"/>
   <wangtile tileid="109" wangid="0,0,0,0,0,2,0,0"/>
   <wangtile tileid="110" wangid="0,2,0,0,0,2,0,2"/>
   <wangtile tileid="111" wangid="0,2,0,2,0,0,0,2"/>
   <wangtile tileid="118" wangid="0,1,0,1,0,0,0,0"/>
   <wangtile tileid="119" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="120" wangid="0,0,0,0,0,1,0,1"/>
   <wangtile tileid="121" wangid="0,0,0,1,0,1,0,1"/>
   <wangtile tileid="122" wangid="0,1,0,1,0,1,0,0"/>
   <wangtile tileid="123" wangid="0,2,0,2,0,0,0,0"/>
   <wangtile tileid="124" wangid="0,2,0,2,0,2,0,2"/>
   <wangtile tileid="125" wangid="0,0,0,0,0,2,0,2"/>
   <wangtile tileid="126" wangid="0,0,0,2,0,2,0,2"/>
   <wangtile tileid="127" wangid="0,2,0,2,0,2,0,0"/>
   <wangtile tileid="134" wangid="0,1,0,0,0,0,0,0"/>
   <wangtile tileid="135" wangid="0,1,0,0,0,0,0,1"/>
   <wangtile tileid="136" wangid="0,0,0,0,0,0,0,1"/>
   <wangtile tileid="137" wangid="0,1,0,0,0,1,0,0"/>
   <wangtile tileid="138" wangid="0,0,0,1,0,0,0,1"/>
   <wangtile tileid="139" wangid="0,2,0,0,0,0,0,0"/>
   <wangtile tileid="140" wangid="0,2,0,0,0,0,0,2"/>
   <wangtile tileid="141" wangid="0,0,0,0,0,0,0,2"/>
   <wangtile tileid="142" wangid="0,2,0,0,0,2,0,0"/>
   <wangtile tileid="143" wangid="0,0,0,2,0,0,0,2"/>
  </wangset>
 </wangsets>
</tileset>
