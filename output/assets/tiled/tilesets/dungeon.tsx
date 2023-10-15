<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.10.2" name="dungeon" tilewidth="16" tileheight="16" tilecount="625" columns="25">
 <image source="dungeon.png" width="400" height="400"/>
 <tile id="0">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="1">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="2">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="25">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="26">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="27">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="50">
  <objectgroup draworder="index" id="3">
   <object id="2" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="51">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="52">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="451">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
  <animation>
   <frame tileid="451" duration="100"/>
   <frame tileid="454" duration="100"/>
   <frame tileid="457" duration="100"/>
  </animation>
 </tile>
 <tile id="454">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="457">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <wangsets>
  <wangset name="ground" type="corner" tile="30">
   <wangcolor name="floor" color="#ff0000" tile="-1" probability="1"/>
   <wangcolor name="floor_meets_wall" color="#00ff00" tile="-1" probability="1"/>
   <wangcolor name="blood" color="#0000ff" tile="-1" probability="1"/>
   <wangtile tileid="4" wangid="0,2,0,1,0,2,0,2"/>
   <wangtile tileid="5" wangid="0,2,0,1,0,1,0,2"/>
   <wangtile tileid="6" wangid="0,2,0,2,0,1,0,2"/>
   <wangtile tileid="29" wangid="0,1,0,1,0,2,0,2"/>
   <wangtile tileid="30" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="31" wangid="0,2,0,2,0,1,0,1"/>
   <wangtile tileid="54" wangid="0,1,0,2,0,2,0,2"/>
   <wangtile tileid="55" wangid="0,1,0,2,0,2,0,1"/>
   <wangtile tileid="56" wangid="0,2,0,2,0,2,0,1"/>
   <wangtile tileid="79" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="80" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="81" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="82" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="284" wangid="0,2,0,2,0,2,0,2"/>
   <wangtile tileid="285" wangid="0,2,0,2,0,2,0,2"/>
   <wangtile tileid="308" wangid="0,2,0,3,0,2,0,2"/>
   <wangtile tileid="309" wangid="0,2,0,2,0,3,0,2"/>
   <wangtile tileid="332" wangid="0,2,0,2,0,2,0,2"/>
   <wangtile tileid="333" wangid="0,3,0,2,0,2,0,2"/>
   <wangtile tileid="334" wangid="0,2,0,2,0,2,0,3"/>
  </wangset>
  <wangset name="moss" type="corner" tile="-1">
   <wangcolor name="moss" color="#ff0000" tile="-1" probability="1"/>
   <wangtile tileid="10" wangid="0,0,0,1,0,0,0,0"/>
   <wangtile tileid="11" wangid="0,0,0,1,0,1,0,0"/>
   <wangtile tileid="12" wangid="0,0,0,1,0,1,0,0"/>
   <wangtile tileid="13" wangid="0,0,0,0,0,1,0,0"/>
   <wangtile tileid="35" wangid="0,1,0,1,0,0,0,0"/>
   <wangtile tileid="36" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="37" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="38" wangid="0,0,0,0,0,1,0,1"/>
   <wangtile tileid="60" wangid="0,1,0,1,0,0,0,0"/>
   <wangtile tileid="61" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="62" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="63" wangid="0,0,0,0,0,1,0,1"/>
   <wangtile tileid="85" wangid="0,1,0,0,0,0,0,0"/>
   <wangtile tileid="86" wangid="0,1,0,0,0,0,0,1"/>
   <wangtile tileid="87" wangid="0,1,0,0,0,0,0,1"/>
   <wangtile tileid="88" wangid="0,0,0,0,0,0,0,1"/>
   <wangtile tileid="110" wangid="0,1,0,0,0,1,0,1"/>
   <wangtile tileid="111" wangid="0,1,0,0,0,0,0,1"/>
   <wangtile tileid="112" wangid="0,1,0,1,0,0,0,1"/>
   <wangtile tileid="113" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="135" wangid="0,0,0,0,0,1,0,1"/>
   <wangtile tileid="137" wangid="0,1,0,1,0,0,0,0"/>
   <wangtile tileid="138" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="160" wangid="0,0,0,1,0,1,0,1"/>
   <wangtile tileid="161" wangid="0,0,0,1,0,1,0,0"/>
   <wangtile tileid="162" wangid="0,1,0,1,0,1,0,0"/>
   <wangtile tileid="163" wangid="0,1,0,1,0,1,0,1"/>
  </wangset>
  <wangset name="wall" type="corner" tile="-1">
   <wangcolor name="wall" color="#ff0000" tile="-1" probability="1"/>
   <wangtile tileid="0" wangid="0,0,0,1,0,0,0,0"/>
   <wangtile tileid="1" wangid="0,0,0,1,0,1,0,0"/>
   <wangtile tileid="2" wangid="0,0,0,0,0,1,0,0"/>
   <wangtile tileid="25" wangid="0,1,0,1,0,0,0,0"/>
   <wangtile tileid="26" wangid="0,1,0,1,0,1,0,1"/>
   <wangtile tileid="27" wangid="0,0,0,0,0,1,0,1"/>
   <wangtile tileid="50" wangid="0,1,0,0,0,0,0,0"/>
   <wangtile tileid="51" wangid="0,1,0,0,0,0,0,1"/>
   <wangtile tileid="52" wangid="0,0,0,0,0,0,0,1"/>
  </wangset>
 </wangsets>
</tileset>
