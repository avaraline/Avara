    �M �M  �VIS3APPL          VIS3VIS4  Over the Hill �.sitst Folderst  PartSIT!                        ��p     ��      W2dmW2df       5� W2dmW2df       6� W2dmW2df       8� W2dmW2df       <� @W2dmW2df       = �                 �Set TagTNAMCountOCNT*****LSTCTagTNAMNameESTRMessageESTRPathESTREnables reqdDWRDFileBOOL	Reserved:HLNGEnables:OCNT-----LSTCLevel:TNAM-----LSTE*****LSTE  e�e�����$b< � ������  ��  <  $b         
����$b<       
 "�� @   � �    � � 
         ���  
 8  ��   "��   ,  	Helvetica    
 .      $`:$`: +A
designer =  )Q+"Cowboy - jmcald@destiny.esd105.wednet.edu"  ( M �information ="Scripting by Hybrid.  Teams must be red, blue or yellow.  Up to 3 people on one team. First team to reach 100 seconds in the lower center square wins." *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
    $`: "���   ��     aAb� � Z #   � �              h � Z � � 
          
OK$]      "���   (XKobject Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (K   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )5blue )Close  (`K   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (�Kend *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )9Blue (\K   n = kTeamScore1  *
   out = @ ))blue )Wins (tKend *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )Wins (@K   showEveryone = true  *   text = "  )%Blue ) team wins!" (XKend *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true */   text = "Yellow team has reached the halfway   *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *.   text = "Yellow team has only 10 seconds to  *go!" *end  � � � � � �  
    $`: "���   ��     a Fk� � Z #   � �              h � Z � � 
          
YW �,      "���   (bWobject Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ))blue )At40 (jW   restart = false  *end  *object Counter *   in = @add1  )9Blue (�W	   n = 50 *
   out = @ ))blue )At50 (�W   restart = false  *end  *object Counter *   in = @add1  )9Blue (�W	   n = 60 *
   out = @ ))blue )At60 (�W   restart = false  *end  *object Counter *   in = @add1  )9Blue (*W	   n = 70 *
   out = @ ))blue )At70 (BW   restart = false  *end  *object Counter *   in = @add1  )9Blue (rW	   n = 80 *
   out = @ ))blue )At80 (�W   restart = false  *end  *object Counter *   in = @add1  )9Blue (�W	   n = 90 *
   out = @ ))blue )At90 (�W   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At10 (W   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 10 seconds!" (>Wend *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 20 seconds!" (�Wend *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 30 seconds!" (�Wend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At40 (:W   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 40 seconds!" (^Wend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At50 (�W   showEveryone = true  *   text = "  )%Blue ) team has reached the halfway  (�Wmark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At60 (�W   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 60 seconds!" (Wend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At70 (ZW   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 70 seconds!" (~Wend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At80 (�W   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 80 seconds!" (�Wend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At90 ( W   showEveryone = true  *   text = "  )%Blue )! team has only 10 seconds to go!"  ( 2Wend � � � � � �  
    $`: "���   ��     a Ed � � Z #   � �              h � Z � � 
          
J O �      "���   (S Oobject Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *,   text = "Red team has reached the halfway  *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
    $`: "���   ��     a \k � Z #   � �              h � Z � � 
          
F)c�      "���   (O)object Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
    $`: "���   ��     a
UV � Z #   � �              h � Z � � 
          
@�%      "���   (Iobject Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
    $`: "��   ��ffff    A �� �� #   � �              H � � 
         ��ffff 1 (� �7         
 (� �7      #     ( 2�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    $`: "���   ����33 A �d �� #   � �              H � � 
         ����33 1 (* ��         
 (* ��      #   ( 2*object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    $`: "��g     ���� A � �G #   � �              H � � 
           ���� 1 (� ��         
 (� ��      #   ( 2�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    $`: "�L��   ������ 	         a��1 � Z #   � �              	�������� h � Z � � 
          
�,�      "�t��    	 (�,unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 120 (I,end *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (�,end � � � � � �  
    $`: "�'�K   � � "�'�w   ������ 	         a��� � Z #   � �              	�������� h � Z � � 
          
���      "�O��   (�unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 120 ($end *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (�end � � � � � �  
    $`: "�(�K   ������ 	         a���� � Z #   � �              	�������� h � Z � � 
          
����      "�P�j   (��unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 120 (%�end *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (��end � � � � � � � �  
    $`: "�$��     ��   a� f � � < #   � �              h � < � � 
          
� ��      "�/�
       (� �object Incarnator *y = 30 *end  � � � � � �  
    $`: "�*��     ��   a��
w < < #   � �              h < < � � 
          
�O��      "�+��   (�Oobject Incarnator *y = 30 *end  � � � � � �  
    $`: "�0��     ��   a� h � � < #   � �              h � < � � 
          
� ��      "�;�   (� �object Incarnator *y = 30 *end  � � � � � �  
    $`: "�6��     ��   a��u < < #   � �              h < < � � 
          
�M��      "�7��   (�Mobject Incarnator *y = 30 *end  � � � � � �  
    $`: "�&��     ��   a� i � � < #   � �              h � < � � 
          
� ��      "�1�   (� �object Incarnator *y = 30 *end  � � � � � �  
    $`: "�1��     ��   a��s < < #   � �              h < < � � 
          
�K��      "�2��   (�Kobject Incarnator *y = 30 *end  � � � � � �  
    $`: "�K�6   ������ 	         a���� � Z #   � �              	�������� h � Z � � 
          
����      "�s�U       	 (��unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 120 (H�end *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (��end � � � � � �  
    $`: "�j�M   � � "�j�y   ������ 	         a��� � Z #   � �              	�������� h � Z � � 
          
��      "����   (unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 120 (gend *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (�end � � � � � �  
    $`: "�k�M   ������ 	         a���� � Z #   � �              	�������� h � Z � � 
          
���      "���l   (�unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 120 (h�end *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (��end � � � � � � ��      
    $`:      A� �yS " �    � �              H � �      "���   � � 
          
���� "��8     (��wa=4  � �     UU  
    $`:    A�� "�3   � �       H � � � �             "��Q   � � 
          
�a�� "���   (�awa=4  � �     UU  
    $`: A���� "��   � �       H � � DD      p p ARy�      "     � �       H A ,y �      #   � �              H ��          A� ���      #   � �       H A���S      #   � �       H A ��S      #   � �       H DD      ` ` A � ,�      #   � �       H � �             "��   ��ff   a   1 y � #   � �       h � � � 
                 
 	  !      "���   (  adjust SkyColor end � �     UU  
    $`: i Z � "  x   � �       h Z � � � 
                 
   )!      "���   *adjust GroundColor end � � � � � �  
    $`: "�5�_   ������ 	         a��� � Z #   � �              	�������� h � Z � � 
          
����      "�=�{    	 (��unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 650 (�end *object Goody +$shape = bspStandardPill *y = 6  *	speed = 2  *
boosters=1 *missiles = 8 *start = mShow  *out = mTaken (��end � � � �     UU  
    $`:    A��S " �    � �      33��    H � �             "�*�   � � #   � � 
          
� � � "�v�     (  �wa=10 � �     UU  
    $`: A� �� � "� �   � �      33��    H � � � �             "�$�   � � 
          
� �� � "�p�    +�wa=10 � �     UU  
    $`: A� �� � "� �   � �      33��    H � � � �             "��   � � 
          
� �� � "�i�   (� �wa=10 � �     UU  
    $`: A� �� � "� �   � �      33��    H � � � � � �             "�%��   � � 
          
��P "�q��   (�wa=10 � �     UU  
    $`: A��S "�.   � �      33��    H � � � �             "�+��   � � 
          
�P "�w��   (wa=10 � �     UU  
    $`: A��S "�.   � �      33��    H � � � �             "�2��   � � 
          
�P "�~��   (wa=10 � �     UU  
    $`: A��S "�.   � �      33��    H � � ����      A��]a      "  &   � �              H ����   A7��a      #   � �              H ����   A�]�      #   � �              H ����   A7��      #   � �              H � �      "���t   #   � �    DD        0[�9a � �             "�#��   ��     a^m	 
 #   � �       h	 
 � � 
                 
�'��      "�#��   (�'object Ramp *   y = 0 *   thickness = 0 *   deltaY = 4  *end  � � � � � � � �  
    $`: "�q��   #   � �       0�_^� � �      "���T   ��     a!Am�c 
 #   � �   @    hc 
 � � 
                 
�T<      "���Y   (�object Ramp *   y = 0 *   thickness = 0 *   deltaY = 4  *end  � � � � � � � �  
    $`: "�F�P   � � 
          
�s�� "�F��   (�swa=4  � �     UU  
    $`: A���� "��   � �       H � � � �             "�F��   � � 
          
���� "�F�8   (��wa=4  � �     UU  
    $`: A�� "�3   � �       H � � � �             "�p�\   � � 
          
G�U� "���\   +$�wa=4  � �     UU  
    $`: A��� "��   � �       H � � � �             "�q�\   � � 
          
H�V� "���\   (R�wa=4  � �     UU  
    $`: A��� "��   � �       H � � � �             "�p�?   � � 
          
G�U� "���?   (Q�wa=4  � �     UU  
    $`: A���� "��   � �       H � � � �             "�q�@   � � 
          
H�V� "���@   (R�wa=4  � �     UU  
    $`: A���� "��   � �       H � � DD      ` ` Ax ,��      "     � �       H � �             "���   #   � �    DD        0[9� � �             "�'�   ��     a#ke� U 
 #   � �       h U 
 � � 
                 
���      "�'�   (��object Ramp +$y = 0 *thickness = 0  *
deltaY = 4 (��end � � � � � � � �  
    $`: "����   #   � �       06_�� � �      "���r   ��     a!_m� � 
 #   � �       h � 
 � � 
                 
>��_      "���|   (H�object Ramp *   y = 0 *   thickness = 0 *   deltaY = 4  *end  � � � � � � � �  
    $`: "�a�`   ������ 	         a��� � Z #   � �              	�������� h � Z � � 
          
����      "�i�|    	 (��unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 650 (>�end *object Goody +$shape = bspStandardPill *y = 6  *	speed = 2  *
boosters=1 *missiles = 8 *start = mShow  *out = mTaken (��end � � � � � �  
    $`: "����   � � #   � � 
          
M�]� "���*     (W�wa = 0  � � ��ffff  
    $`: 1]a6� "W�   � �              8 � � 
          
���V      "� �3   +	3object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (��end � � � � � �  
    $`: "����   � � 
          
R�b� "���K   (\�wa = 0  � � ����33  
    $`: 9 "\�   � �              8 � � 
          
���@      "��7   (��object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (��end � � � � � �  
    $`: "����   � � 
          
R�b� "���   (\�wa = 0  � �   ����  
    $`: 9 "\�   � �              8 � � 
          
���      "��   +3object Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (��end � � � � � � � �  
    $`: "�H�   #   � �       `��� � Z � � 
          
���      "�~�    	 (�object Teleporter *
group = -1 *y = -30  *activeRange = 250  *deadRange = 100  *destGroup = @recenter  *	shape = 0  *end  *object Teleporter  *y = 30 *	shape = 0  *group = @recenter  *end  � � � � � �  
    $`: "���u   ����33 aR�� i < #   � �              h i < � � 
          
p
��      "����     (z
object Walker +$lives = kDefaultLives *team = 6 *y = 9  (�
end � � � � � �  
    $`: "����   ����33 aJ` i < #   � �              h i < � � 
          
,Tr�      "����   (6Tobject Walker +$lives = kDefaultLives *team = 6 *y = 9  (nTend � � � � � �  
    $`: "�b�1   ����33 a�� � i < #   � �              h i < � � 
          
��(n      "�b�F   (��object Walker +$lives = kDefaultLives *team = 6 *y = 9  ($�end � � � � � �  
    $`: "���   ����33 aP �� � < #   � �              h � < � � 
          
� 7� �      "��   (� 7object Walker +$lives = kDefaultLives *team = 3 *y = 9  (� 7end � � � � � �  
    $`: "�`��   ����33 a� ��A � < #   � �              h � < � � 
          
� e8      "�r��   +.9object Walker +$lives = kDefaultLives *team = 3 *y = 9  (4 eend � � � � � �  
    $`: "���N   ����33 afJ� � < #   � �              h � < � � 
          
? ��v      "���N   +iobject Walker +$lives = kDefaultLives *team = 3 *y = 9  (� �end � � � � � �  
    $`: "�
��   ����33 a��E < #   � �              h < � � 
          
� _�      #   (� _object Walker +$lives = kDefaultLives *team = 2 *y = 9  (� _end � � � � � �  
    $`: "���A   ����33 a?c{� < #   � �              h < � � 
          
9 �i      #   (C �object Walker +$lives = kDefaultLives *team = 2 *y = 9  ({ �end � � � � � �  
    $`: "��   ����33 a �A  < #   � �              h < � � 
          
� :E �      #   (	 :object Walker +$lives = kDefaultLives *team = 2 *y = 9  (A :end � � � � �  [�[�����#�w � ������  ��  w  #�         
����#�w       
 "�� @   � �    � � 
          
 :  �   "��   ,  	Helvetica    
 .      #�u#�u +C
designer =  )Q+"Cowboy - jmcald@destiny.esd105.wednet.edu"  ( O �information ="Scripting by Hybrid.  Teams must be red, blue or yellow.  Up to 3 people on one team. First team to reach 100 seconds in the center square wins." *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � � 
          
 � " � "�a�   , 	 Geneva    +<gravity = 0.9 *grenadePower = 1 *missilePower = 2 *missileTurnRate = 1  *missileAcceleration = 1  � � � �  
    #�u "�_��   � � "�d��   � � 
          
�8�u "�d��       (�8wa = 2.5  � � ����33  
    #�u    A��� "�D   � �      ����    H � � 
                 
 e�      "����   ( object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (` end � � � � � �  
    #�u "�`��   � � 
          
�p�� "�`��   (�pwa =2.5 � �   ����  
    #�u A��� "�B   � �       H � � 
                 
%n�      "����   (%object Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (i%end � � � � � �  
    #�u "�_��   � � 
          
�y�� "�_��   (�ywa = 2.5  � � ��      
    #�u A��� "�C   � �      ����    H � � 
                 
�^�      "�{��   (object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (Yend � � � � � �     UU  
    #�u    A��� " _    � �      ����    H � �             "��     ff�� a   1 y Z � #   � �        UU    h Z � � � 
                 
  ~ (       "���      
 ( % ~adjust GroundColor end  � � � � � �  
    #�u "���   ��     aI|�� � Z #   � �              h � Z � � 
          
��#�Z      "�     (��object Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (J�   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )5blue )Close  (��   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (.�end *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )9Blue (��   n = kTeamScore1  *
   out = @ ))blue )Wins (��end *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )Wins (r�   showEveryone = true  *   text = "  )%Blue ) team wins!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true */   text = "Yellow team has reached the halfway   *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *.   text = "Yellow team has only 10 seconds to  *go!" *end  � � � � � �  
    #�u "��   ��     aR��� � Z #   � �              h � Z � � 
          
���g      "�   (��object Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ))blue )At40 (��   restart = false  *end  *object Counter *   in = @add1  )9Blue (��	   n = 50 *
   out = @ ))blue )At50 (��   restart = false  *end  *object Counter *   in = @add1  )9Blue (�	   n = 60 *
   out = @ ))blue )At60 (,�   restart = false  *end  *object Counter *   in = @add1  )9Blue (\�	   n = 70 *
   out = @ ))blue )At70 (t�   restart = false  *end  *object Counter *   in = @add1  )9Blue (��	   n = 80 *
   out = @ ))blue )At80 (��   restart = false  *end  *object Counter *   in = @add1  )9Blue (��	   n = 90 *
   out = @ ))blue )At90 (�   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At10 (L�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 10 seconds!" (p�end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 20 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 30 seconds!" (0�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At40 (l�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 40 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At50 (��   showEveryone = true  *   text = "  )%Blue ) team has reached the halfway  (��mark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At60 (,�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 60 seconds!" (P�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At70 (��   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 70 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At80 (��   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 80 seconds!" (�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At90 (L�   showEveryone = true  *   text = "  )%Blue )! team has only 10 seconds to go!"  (d�end � � � � � �  
    #�u "��   ��     aK��� � Z #   � �              h � Z � � 
          
|��U      "��
   (��object Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *,   text = "Red team has reached the halfway  *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
    #�u "���   ��     aC^�� � Z #   � �              h � Z � � 
          
xgD�      "���   (�gobject Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
    #�u "���   ��     a<F�� � Z #   � �              h � Z � � 
          
rQ�`      "���   ({Qobject Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
    #�u "��V   ��ffff    A  6 #   � �              H � � 
         ��ffff 1Z� r         
Z� r      #     (d�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    #�u "���   ����33 A 
� !� #   � �              H � � 
         ����33 1Ze          
Ze       #   (deobject WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    #�u "���     ���� A 	Y  � #   � �              H � � 
           ���� 1Z" �         
Z" �      #   (d"object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    #�u "�(��   ��        A�w� #   � �              H � � 
          
����      "�1�      
 (��object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � �     UU  
    #�u A�7| " ?    � �              H � � 
          
�6y      "�>��   (�6object Field  *
visible =  *true *
deltaY = 0 *
deltaZ = 0 *end  � � ��      
    #�u    A�) " L    � �              H � � 
          
PU      "����   (object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � ��      
    #�u A�	) " ?    � �              H � � 
          
�Q      "�X��   (�object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � �  
    #�u "��   ����   a   1 y � #   � �      ����    h � � � 
                 
 	 ~        "���   (  ~adjust SkyColor end � � � �     UU  
    #�u A��� "  a   � �              H � � 
          
�P      "���>   (�object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � �     UU  
    #�u A��	� " ?    � �              H � � 
          
��      "�X�:   (��object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � �  
    #�u "�@��     33�� a�U�� � < #   � �              h � < � � 
          
�}=�      "�H��   (�}object Incarnator *       )y = 0  (�}end *object Teleporter  *     y = 3.5 *     shape = 0 *     group = @safety *end  � � � � � �  
    #�u "�E��     33�� a�C�  < #   � �              h  < � � 
          
�l:�      #   (�lobject Incarnator *
     y = 0 *end  *object Teleporter  *     y = 3.5 *     shape = 0 *     group = @safety *end  � � � � � �  
    #�u "�#��     33�� a�S�' < #   � �              h' < � � 
          
�O�      #   (�Oobject Incarnator *       )y = 0  (�Oend *object Teleporter  *     y = 3.5 *     shape = 0 *     group = @safety *end  � � � � � �  
    #�u "�@��     33�� a�C�� i < #   � �              h i < � � 
          
�l=�      "�H��   (�lobject Incarnator *
     y = 0 *end  *object Teleporter  *     y = 3.5 *     shape = 0 *     group = @safety *end  � � � � � �  
    #�u "���     33�� aoL�� � < #   � �              h � < � � 
          
�t�      "�$��   (�tobject Incarnator *
     y = 0 *end  *object Teleporter  *     y = 3.5 *     shape = 0 *     group = @safety *end  � � � � � �  
    #�u "�i��     33�� a�LB�J < #   � �              hJ < � � 
          
�t^�      "�i��   (�tobject Incarnator *
     y = 0 *end  *object Teleporter  *     y = 3.5 *     shape = 0 *     group = @safety *end  � � � � � �  
    #�u "���"   ��     AT�gR #   � �              H � � 
          
@��;      "���q   (J�object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � � � �  
    #�u "���"       UU AJ�]R #   � �              H � � 
          
=i~�      "����   (Giobject Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � � � �  
    #�u "���T   � � "���t   ����33 a>��:J < #   � �              hJ < � � 
          
F���      "���w       (P�object Walker +$lives = kDefaultLives *team = 3 *y = 30 (��end � � � � � �  
    #�u "����   ����33 a>K��J < #   � �              hJ < � � 
          
Fg�      "����   (Pgobject Walker +$lives = kDefaultLives *team = 3 *y = 30 (�gend � � � � � �  
    #�u "���T   ����33 a>��J < #   � �              hJ < � � 
          
F��      "���W   (P�object Walker +$lives = kDefaultLives *team = 3 *y = 30 (��end � � � � � � � �  
    #�u "�8�B   � � "�8�D   ����33 a�V�� � < #   � �              h � < � � 
          
��l      "�D�D   (��object Walker +$lives = kDefaultLives *team = 2 *y = 30 (�end � � � � � �  
    #�u "���B   ����33 a6T{� � < #   � �              h � < � � 
          
T��j      "���B   (^�object Walker +$lives = kDefaultLives *team = 2 *y = 30 (��end � � � � � �  
    #�u "�X�C   ����33 a�U� � < #   � �              h � < � � 
          
��4k      "�d�C   +Xobject Walker +$lives = kDefaultLives *team = 2 *y = 30 (&�end � � � � � � � �  
    #�u "�1��   � � "�Q��   ����33 a�A
� < < #   � �              h < < � � 
          
��<      "�Q�   (��object Walker +$lives = kDefaultLives *team = 6 *y = 30 (�end � � � � � �  
    #�u "�1��   ����33 a�A�� < < #   � �              h < < � � 
          
���<      "�1�   (��object Walker +$lives = kDefaultLives *team = 6 *y = 30 (��end � � � � � �  
    #�u "����   ����33 a6B{� < < #   � �              h < < � � 
          
B��=      "���   +Yobject Walker +$lives = kDefaultLives *team = 6 *y = 30 (��end � � � � � � � �  
    #�u "����   ��  �� a%H�� � Z #   � �              h � Z � � 
          
]��      "����      
 (gobject Teleporter *
group = -1 *y = -30  *activeRange = 200  *deadRange = 175  *destGroup = @safety  *	shape = 0  *end  � � � � � �  
    #�u "����   ������ 	         a>_r� � Z #   � �              	�������� h � Z � � 
          
e�O�      "���   (o�unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *	wait = 75  (��end *object Goody +$shape = bspStandardPill *y = 4.5  *	speed = 2  *grenades=12  *missiles = 8 *start = mShow  *out = mTaken (L�end � � � �     UU  
    #�u al��= � 
 " �    � �    ��        h � 
 � �             "���   #   � �       0��� � � 
          
�c��      "���   (�cobject Ramp *   y = 0 *   thickness = 0 *   deltaY = 2.5  *end  � � � � � �  
    #�u "�?��       UU a��D=c 
 #   � �   @ ��        hc 
             #   � �       0� � � � � � 
          
�]�      "�@��   (�]object Ramp *   y = 0 *   thickness = 0 *   deltaY = 2.5  *end  � � � �  
    #�u "�u�;   #   � �    DD        0��� � �             "���;       UU a��E� U 
 #   � �    ��        h U 
 � � 
                 
�QI      #   (�object Ramp +$y = 0 *thickness = 0  *deltaY = 2.5 (N�end � � � � � � � �  
    #�u "��?   ������  8 8 A��� #   � �       H � � 
                 
���(      #   (��object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � � � �  
    #�u "�u�W   #   � �    DD        0���2     UU aa�0M	 
      #   � �    ��        h	 
 � � 
                 
"�dp      "���l   (,�object Ramp *   y = 0 *   thickness = 0 *   deltaY = 2.5  *end  � � � � � �  
    #�u "�?�W   ������ A���� #   � �       H � � 
                 
��S      "�M�i   (��object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � � � �  
    #�u "�.�V   ������ A���� #   � �       H � � 
                 
���G      "�.�^   (��object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � � � �  
    #�u "�?�F   ������ A��� #   � �       H � � 
                 
��	/      "�H�F   (��object Field  *visible = true *
deltaY = 0 *
deltaZ = 0 *end  � � � � �  ]]����<�� � ������  ��  �  <�         
����<��       
 "�� @   � �    � � 
         ���  
 � Q �   "��   , 	 Geneva     
 .      <��<�� +�gravity = 0.25  *grenadePower = 3 * missilePower = 2  *missileTurnRate = 1  *missileAcceleration = .5 *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
    <�� "%��9   ��     a2E�2� � Z #   � �              h � Z � � 
          
2}�<��      "%��B   ,  	Helvetica   (2��object Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (3F�   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )5blue )Close  (3��   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (4*�end *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )9Blue (4��   n = kTeamScore1  *
   out = @ ))blue )Wins (4��end *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )Wins (5n�   showEveryone = true  *   text = "  )%Blue ) team wins!" (5��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true */   text = "Yellow team has reached the halfway   *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *.   text = "Yellow team has only 10 seconds to  *go!" *end  � � � � � �  
    <�� "%��>   ��     a2N�2� � Z #   � �              h � Z � � 
          
2��8��      "&�N   (2��object Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ))blue )At40 (3��   restart = false  *end  *object Counter *   in = @add1  )9Blue (3��	   n = 50 *
   out = @ ))blue )At50 (3��   restart = false  *end  *object Counter *   in = @add1  )9Blue (4�	   n = 60 *
   out = @ ))blue )At60 (4(�   restart = false  *end  *object Counter *   in = @add1  )9Blue (4X�	   n = 70 *
   out = @ ))blue )At70 (4p�   restart = false  *end  *object Counter *   in = @add1  )9Blue (4��	   n = 80 *
   out = @ ))blue )At80 (4��   restart = false  *end  *object Counter *   in = @add1  )9Blue (4��	   n = 90 *
   out = @ ))blue )At90 (5 �   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At10 (5H�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 10 seconds!" (5l�end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 20 seconds!" (5��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 30 seconds!" (6,�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At40 (6h�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 40 seconds!" (6��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At50 (6��   showEveryone = true  *   text = "  )%Blue ) team has reached the halfway  (6��mark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At60 (7(�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 60 seconds!" (7L�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At70 (7��   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 70 seconds!" (7��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At80 (7��   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 80 seconds!" (8�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At90 (8H�   showEveryone = true  *   text = "  )%Blue )! team has only 10 seconds to go!"  (8`�end � � � � � �  
    <�� "%��=   ��     a2G �2� � Z #   � �              h � Z � � 
          
2x �8��      "%��F   (2� �object Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *,   text = "Red team has reached the halfway  *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
    <�� "%��   ��     a2?�2�� � Z #   � �              h � Z � � 
          
2t�6�2      "%��    (2}�object Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
    <�� "%��   ��     a28�2�� � Z #   � �              h � Z � � 
          
2n�4��      "%��   (2w�object Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
    <�� "+���   ��ffff    A9I9r #   � �              H � � 
         ��ffff 18V8��         
8V8��      #     (8`object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    <�� "+��!   ����33 A9�9 #   � �              H � � 
         ����33 18V�8�=         
8V�8�=      #   (8`�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    <�� "+���     ���� A9�9� #   � �              H � � 
           ���� 18V^8��         
8V^8��      #   (8`^object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    <�� "�}��   � � 
          
 ��� "�}�0   (�wa=20 � � ������  
    <��    Aa	\ "�   � �              H � � � �      "�I�h   � � 
          
 � "���h   (
�wa = 0  � � ����33  
    <��  � � A�Y� "�   � �              H � � 
          
�K�      "�[��   (�object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (9end � � � � � �  
    <�� "�H�p   � � 
          
�� "�v�p   ( �wa = 0  � � ��ffff  
    <�� A�X� "�   � �              H � � 
          
�Q�      "�a��   (�object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (?end � � � � � �  
    <�� "����   ������ 	         a�L"q � Z #   � �              	�������� h � Z � � 
          
i�A      "����    	 ( iunique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *wait = 5 (miend *object Goody +$shape = bspStandardPill *y = 16 *	speed = 2  *grenades=12  *missiles = 8 *
boosters=5 *start = mShow  *out = mTaken (�iend � � � � � �  
    <�� "�I��   � � 
          
��� "��     (	�wa = 0  � �   ��    
    <�� A�Y� "�:   � �              H � � 
          
�F�      "�d��   (�object Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (Bend � � � � � �  
    <�� "���1   � � "��I   � � 
          
D "����   (wa=6  � � ��      
    <��    A��x "��   � �              H � � � �      "��1   � � 
          
���0 "�8�n   (��wa=6  � � ��      
    <�� A��x� "��   � �              H � � � �      "���1   � � 
          
PG^� "����   (ZGwa=6  � � ��      
    <�� AU�� "Z�   � �              H � � � �      "���1   � � 
          
�}�� "�7��   (�}wa=6  � � ��      
    <�� Aw�� "��   � �              H � � � � � �      "���-   � � #   � � 
          
[Ci� "����   (eCwa=6  � �   ��    
    <�� A`�� "e�   � �              H � � � �      "�(�E   � � 
          
�(@ "���~   +��wa=6  � �   ��    
    <�� A��� "��   � �              H � � � �      "�(�-   � � 
          
���, "�C�j   (��wa=6  � �   ��    
    <�� A���� "��   � �              H � � � �      "��-   � � 
          
�y�� "�B��   +��wa=6  � �   ��    
    <�� A��� "��   � �              H � � � � � � 
          
 2  O>      "��   ( < 
designer =  )Q+"Cowboy - jmcald@destiny.esd105.wednet.edu"  ( J information = )Q"   
 )Design ) � by Cowboy, Scripting by Hybrid.  Special thanks to my good buddy Taz!  Teams must be red, yellow or blue.  Up to 3 people on one team.  Stay in the square anywhere between the floor and the ceiling for 100 seconds.    ( J>" � �  
    <�� a   - u � " >   � �       h � � � 
          
  z       "���   (  zadjust SkyColor end � �     UU  
    <�� i Z � "  p   � �    ��        h Z � � � 
                 
  z %      "���   *adjust GroundColor end � � ������  
    <��  � � AZ "  �   � �              H ������ A��      #   � �              H ������ A�`      #   � �              H ������ Ac	�      #   � �              H � �      "���1   � � "�(�1   � � 
          
���1 "�D�o   (��wa=6  � � ����    
    <��    A���� "��   � �              H � � � �      "���1   � � 
          
\Hj� "����   (fHwa=6  � � ����    
    <�� A`�� "f�   � �              H � � � �      "�(�I   � � 
          
(D "����   +��wa=6  � � ����    
    <�� A��� "��   � �              H � � � �      "��1   � � 
          
�~�� "�C��   (�~wa=6  � � ����    
    <�� A��� "��   � �              H � � � � � �      "���-   � � "��E   � � 
          
�@ "���~   (�wa=6  � � ��  ��  
    <�� A��x "��   � �              H � � � �      "��-   � � 
          
���, "�8�j   (��wa=6  � � ��  ��  
    <�� A��x� "��   � �              H � � � �      "���-   � � 
          
PC^� "����   (ZCwa=6  � � ��  ��  
    <�� AU�� "Z�   � �              H � � � �      "���-   � � 
          
�z�� "�7��   (�zwa=6  � � ��  ��  
    <�� Aw�� "��   � �              H � � � � � �      "���     ��   aA�� < #   � �              h < � � 
          
_��$      "���,     (i�object Incarnator *y = 7  *end  � � � � � �  
    <�� "��o     ��   a]��3 i < #   � �              h i < � � 
          
� ��      "���   , 	 Monaco    (� object Incarnator *y = 7  *end  � � � � � �  
    <�� "���l     ��   a?��0  < #   � �              h  < � � 
          
O��      "����   ([object Incarnator *y = 7  *end  � � � � � �  
    <�� "��     ��   a]�� � < #   � �              h � < � � 
          
���P      "�+�   (��object Incarnator *y = 7  *end  � � � � � �  
    <�� "����     ��   aG��J < #   � �              hJ < � � 
          
O9��      "����   ([9object Incarnator *y = 1  *end  � � � � � �  
    <�� "���     ��   aP�� � < #   � �              h � < � � 
          
�E��      "�!��   (�Eobject Incarnator *y = 1  *end  � � � � � �  
    <�� "���5   � � "���U   ����33 ab��J < #   � �              hJ < � � 
          
j���      "���X        (t�object Walker +$lives = kDefaultLives *team = 6 *y = 0  (��end � � � � � �  
    <�� "����   ����33 ab,��J < #   � �              hJ < � � 
          
jH��      "����   (tHobject Walker +$lives = kDefaultLives *team = 6 *y = 0  (�Hend � � � � � �  
    <�� "���5   ����33 ab���J < #   � �              hJ < � � 
          
j��`      "���8   (t�object Walker +$lives = kDefaultLives *team = 6 *y = 0  (��end � � � � � � � �  
    <�� "���h   ������ 	         a��$ � Z #   � �              	�������� h � Z � � 
          
��      "����       ��  	 (8+object And in = @start out = @makeammo3 end *object Timer +$in = @switchagain2  *out = @makeammo3 *	wait = 75  (zend *object Goody +$shape = bspMissile  *y = 2  *	speed = 2  *missiles = 8 *start = @makeammo3 *out = @switchammo2 (�end � � � � � �         
    <�� "���   ������ 	         a�M�r � Z #   � �              	�������� h � Z � � 
          
�mkE      "�F��       �� (�mobject Timer  +$in = @switchammo2 *out = @makeammo4 *	wait = 75  (�mend *!object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = @makeammo4 *out = @switchagain2  (hmend � � � � � �         
    <�� "���2   ������ 	         a��$� � Z #   � �              	�������� h � Z � � 
          
���      "���N       �� (8�+object And in = @start out = @makeammo1 end *object Timer +$in = @switchagain1  *out = @makeammo1 *	wait = 75  (z�end *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = @makeammo1 *out = @switchammo1 (��end � � � � � �         
    <�� "���   ������ 	         apN�s � Z #   � �              	�������� h � Z � � 
          
�nPF      "�+��       �� (�nobject Timer  +$in = @switchammo1 *out = @makeammo2 *	wait = 75  (�nend *!object Goody +$shape = bspMissile  *y = 2  *	speed = 2  *missiles = 8 *start = @makeammo2 *out = @switchagain1  (Mnend � � � � � �         
    <�� "�I��   � � 
          
S� "����       ww   (]floorTemplate = bspFloorFrame *wa = 0 *wallHeight = 0 *wallYon = 40 � �         
    <�� 1�Y� "�]   � �       8 � � � �      "����   ����33 a��,� � < #   � �              h � < � � 
          
!g�      "����   (+object Walker +$lives = kDefaultLives *team = 3 *y = 7  (cend � � � � � �  
    <�� "���   ����33 a��� � < #   � �              h � < � � 
          
���      "�0��   (�object Walker +$lives = kDefaultLives *team = 3 *y = 7  (�end � � � � � �  
    <�� "��>   ����33 ayV�� � < #   � �              h � < � � 
          
���f      "�*�>   (��object Walker +$lives = kDefaultLives *team = 3 *y = 7  (��end � � � � � �  
    <�� "���l   ����33 a��- i < #   � �              h i < � � 
          
U�      "����   (object Walker +$lives = kDefaultLives *team = 2 *y = 7  (Qend � � � � � �  
    <�� "���   ����33 a}4�x i < #   � �              h i < � � 
          
�l�      "���   (�lobject Walker +$lives = kDefaultLives *team = 2 *y = 7  (�lend � � � � � �  
    <�� "� �r   ����33 a��� i < #   � �              h i < � � 
          
���      "� ��   (�object Walker +$lives = kDefaultLives *team = 2 *y = 7  (�end � � � � � A�A����� �� � ������  ��  �   �         
���� ��      	w���w��� "�� @   � �    �� 8BIM       ��Gr�ph�bj  
     �� �   ��      ��        H   H                        ��     �� @ �)�� � ���  �    �  �� ��!)11)�����)�� ���  �  �  �� ��	!))1)����)����  �  �  �  � �� ��	!))1)����������")���  �� �  �  �  �  �  � � ���!)!)!!�� � �� � � ��� � �� � �� ��� �� �������� �� ����� � � � �� ��� � �� � � ��� � �� �
����)���  �  �  �  �  �  �  � ��� !�)!!�����)���  �  �  �  �  �  �  � ��� !�)!!���������� � )��� ��  �  � � �  �  ���	!)!!� !���� )����  �  �  �� �  �  �� �	!)!!� !����� )����  �  �� �  �  ���	!)!!�!����������� �)�� ����  �    �  �� �� �!))����)�� ���  �  �  �� �� ��!!)!�����)����  �  �  �  � �� ��!!)!���������� �)���  �  �  �  �  �  �  � � ���!)!)!!����)���  �  �  �  �  �  �  � ��� !�)!!�����)���  �  �  �  �  �  �  � ��� !�)!!����������! )����  �  �� �  �  ���	!)!!� !���� )����  �  �� �  �  �� �	!)!!� !�� � � � � � � �� ���� � � � � � � ���� �� �� � � ���������� ��� � ��� � � � � � � �� �� �� � )����  �  �� �  �  ���	!)!!�!���� �ʵ �ݵ ��� ��� ��� �� ��� ��� ��� ��� ��� ��� ��� �� �� ��� ��� ��� �� ��� ��� �� �� �ʵ �� �� ��� �C )�����  �� � � �� ���!))���� )�����  �� � � �� �� ��!!)!���� )�����  �� � � ���!!)!���������� ��������� ��� ����������������������������� �����
������������� ������� ���������� ���������
������������� �����������������������������������������������������	����������������������� ��� ���������������������������������������������������������������������������������� ��������� ��� ����������������������������� �����
������������� �������������������������������.)�����  �  ��  � �� � ���!)!)!!����)�����  �  ��  � �� � ��� !�)!!����)�����  �  ��  � �� � ��� !�)!!���������������������������������� ��� ��� ��� �� ��� ��� ��� ���������������� ��� ������������ ��� �������������� ������� ��� ��� �������������������������� ���������������������������� ���������������� ����������� ��� ��� ��� ���������������������������������� ��� ��� ��� �� ��� ��� ��� ������������� ��� � �)� ����  �� ��   �  �  � � �	!)!!�����)� ����  �� ��   �  �  � � �	!)!!�����)� ����  �� ��   �  �  � � �	!)!!���������~ !������ ��  � � � �  �  � ���! �!�!!�!�! �!��! �!!!�!��! �! �! �! �! �!!�!!!��! �! �! �! �!!!�! �!!!�! �! �! �! �! �!�!!!�! �! �! �!�!!�!�! �!��! �!!!!�! �!!!!!!!!������ ��  � � � �  �  � ����� !������ ��  � � � �  �  � ���������� �!�����  �   �   � �      �	!� �!�!�!�!�����  �   �   � �      �	!� ����!�����  �   �   � �      �	!� �������� � !���  � �    �	    � ����!�!�!�!���  � �    �	    � ������� !���  � �    �	    � ������������ !���  � � � �  �  �  �  �� ���!�!�!�!���  � � � �  �  �  �  �� ��� �� ��� � ��� �� � � �� � �� ����� ���� � ���� � � �� ��� � ��� �� � � � � � !���  � � � �  �  �  �  �� ���������� �!����  �  � �  �   � �  �  �  �������!�!�!�! ����  �  � �  �   � �  �  �  ����������!����  �  � �  �   � �  �  �  ���������������� �  �� ��  �         �  � �� �  ���� �!�!�!�!�� �  �� ��  �         �  � �� �  ���� ������ �  �� ��  �         �  � �� �  ���� ����������  �� � �  � �   �   �  �  � ��� � �!�!�!�!��  �� � �  � �   �   �  �  � ��� � ������  �� � �  � �   �   �  �  � ��� � ��������L����  �  �  �� �� �  ��   �  �
��  �)�!))�!
)!))!!))!!)�! )�!)!)!)�!)!)!))!�)!!)�!))!)!!)�!)!!)!!)�!�)!))!)!!))�!))!!))!)�! )�!)!!�)�! )�!)!))�!))�! )�!)!!�)�!)!)!!)�!))�!))!!))�!))!))�!))!)!)�!�)!!)�!)!)!)!)!!))!�)!)!)�!)!)�!))!))!!�)�!)!!))�!))!!)!!))�!))�! )�! )�! )�!�)�!))�!
)!))!!))!!)�! )�!)!)!)�!)!)!))!�)!!)�!))!)!!)�!)!!)!!)�!�)!))!!�)�!�) !�)!)�!�)�!�)!!����  �  �  �� �� �  ��   �  �
��  ���� ����  �  �  �� �� �  ��   �  �
��  ��������!��  �  BR1��  �   � � �  �� �  ����  �  �)�)�)�)!��  �  BR1��  �   � � �  �� �  ����  �  ����!��  �  BR1��  �   � � �  �� �  ����  �  ��������S!��  �  � Z{B �  ��  )9Jk���s1!�  �� � �����  �  �)�)�)�)!��  �  � Z{B �  ��  )9Jk���s1!�  �� � �����  �  ����!��  �  � Z{B �  ��  )9Jk���s1!�  �� � �����  �  ��������� !� ��  �  !ZB �!))1k���������{c9�  �   �����  �)ZkZ��cc��k���)k���)�)9ccZZR�) B�)�) !� ��  �  !ZB �!))1k���������{c9�  �   ����� �!	!!!!�!�!�! �! �!!�!!�!!!!!!!!��! �! �!ZcR��RZ��c��!�! �!c��!!�!�!!!�!�!!!!!�! �!��!!�! �! �! �!!�!!�! �!�! �!!!�!)RZRRJ�!!!!!!�!!!�!!!�! 9�! �!	!!!!�!�!�! �! �!!�!!�!!!!!!!!��! �! �!!�! �! �!!!!�!!!� ��  �  !ZB �!))1k���������{c9�  �   �����  �����������������ށ�� ���ý Ɓ��� !� �� �  �  �	 J!)1)���  �9BBR����{scRB1BZ�R1!)!!� ���  �)����{{��{��{B�)9�R�)�� �){k�)c�����9�)���� ��){k�)����ޔ�)�) !� �� �  �  �	 J!)1)���  �9BBR����{scRB1BZ�R1!)!!� ���  �!����{{��{��{B�!)�J�!�� �!sc�!Z�����)�!���� ��!sc�!����ޔ�!�!� �� �  �  �	 J!)1)���  �9BBR����{scRB1BZ�R1!)!!� ���  ������{{��{��{{�������� Z������������Ͻ�����ܽ�����ށ��� !��  �  )�! ��!99Zck��91))!!!)1BJB�)9J1 � ��  �) �� ))��Z!�)�R  �){���){��RB1 !�)�1�  �){���)c�{!�  !�)�) !��  �  )�! ��!99Zck��91))!!!)1BJB�)9J1 � ��  �! �� !��Z�!�R  �!s���!s��RB1 �!�1�  �!s���!Z�{!�  �!�!��  �  )�! ��!99Zck��91))!!!)1BJB�)9J1 � ��  �� �� s����{����R �����������RB1 )�н�1�  9�����޽��{!�  c������  ��  !)��  ��)JR9Z��{� �)R99JJZcJBB)!)9c�c��  �) � )�c !�)�B  !�)Μ !�)9k� !�) !�  �)Μ !�)9��  �)�)��  ��  !)��  ��)JR9Z��{� �)R99JJZcJBB)!)9c�c��  �! � !�c �!�B  �!Μ  �!1k�  �! � �!Μ  �!1��  �!�!��  ��  !)��  ��)JR9Z��{� �)R99JJZcJBB)!)9c�c��  ��R� B����c R���B  ���� !�ƌ�  BϽ c�  ��� !޽Ƶ�  ��������� 1 �( BB99�έ9)!))1BRcR1)!9)!)1B)J�{9� �� �)!� !)��  �){{  !�)�B  !�) �  �)1� �)�B  !�) � �)�)��� 1 �( BB99�έ9)!))1BRcR1)!9)!)1B)J�{9� �� �!� !��  �!s{  �!�B  �! �  �!�  �!�B  �! �  �!�!��� 1 �( BB99�έ9)!))1BRcR1)!9)!)1B)J�{9� �� �� B�  Z�����  )����{ ����B  ���J� )�Ͻ�!� s���B  �޽�1�  ������ ��� ����  )9)R��kB1!B)1k��B)�1!1JRB1!)��B!�� �  �1 �� )11s�  )�1c� !�1J�B  !�1 k� )�1�1� �1J�B  !�1J� �1�1 ��� ����  )9)R��kB1!B)1k��B)�1!1JRB1!)��B!�� �  �! �� �!k�� �!Z�  �!B�B  �! c� �!�)�  �!B�B  �! B�  �!�! ��� ����  )9)R��kB1!B)1k��B)�1!1JRB1!)��B!�� �  �� �� ������  B���Μ J�����B ���� )�ν�B� Z����B �۽ � ������{ � ����!1!1Z�ƔR11!199��ksZ�R�s���ccJ9B9!!BJB1�� �1�1� !11Z��� ZZ�19� )11s���9�19cc9�1Jcs�Z�19c9�19��  Jc�1c{� )�1Zc�19c9�1c{� �19��  Jc�1ZZ�1c�� 11JJ�1�1 � ����!1!1Z�ƔR11!199��ksZ�R�s���ccJ9B9!!BJB1�� �!�1� !!J��� RJ�!1�  �! k���1�!1ZZ1�!BZk�R�!1Z1�!1��  BZ�!Z{� �!RZ�!1Z1�!Z{�  �!1��  BZ�!RR�!Z�� !!BB�!�! � ����!1!1Z�ƔR11!199��ksZ�R�s���ccJ9B9!!BJB1�� ���1� )������  )�����ƽ Z��� ��������������������������� �����{� )�����������{� {������ ��������Μ� Z����Ɓ��� !������� !)Z�ƭc)!)!Bk��RJZ99JsZ�1!))B1Bkk1!!�  �� �1�B� )11��� J���1� 11s���B����{�1��ｭ91s�޽�{�J�19{��ﵽΜ�1��1� )���1c�� {���Μ9�1c��νޜs�1J�����{Z���k�1	B������1��� �1��1� )���1Z���ενZ�1 �� �Z��c�1s���Υ{�1 !������� !)Z�ƭc)!)!Bk��RJZ99JsZ�1!))B1Bkk1!!�  �� ))!)!)!))!))!!�) !�) !�)!))!�) !�)�!)!!�) !�)!!�) !�)!)!)!!�) !�)!))!�)�B� !))��� J��))!)� ))k���B{���{!!))!!))!�)��ｭ1)k�޽�{�B�)1{��ﵽΔ�)��1� )��))�!)!�)!)!))Z�� 	s���Δ1)!�)!Z��ν֔k!!)!)B�����{R���c�)	1������)��� !!�) !�)!!��1� )��!�)	R���ένR!�) !�) !�)!)!!))�� {R��Z!)!)!))!)k���Υ{�) !�)!))!�) !�)�!)!!�) !�)!!�) !�)!)!)!!�) !�)!))!�) !�)!))!�) !�)�!)!�)!)!!������� !)Z�ƭc)!)!Bk��RJZ99JsZ�1!))B1Bkk1!!�  �� ���B� )������ Z��� {�������B��������ｽ����޽�{����������������1� B���ν� Z������������ν�������������������	������޽�� {�����1� B��������������� )�������������֊�� !���� �����)19)!c���B!!)Jk���{c1))RJ1!)1!BB�91� � �1�B� )1s�B�  �1� 1c��� cc!�1�� J��� )�1Z��B  1BkB�1J�{�  �1c�� �{  BB�1J���B� ){9�1J�11 B��c  k11J��{) c��c� �1J�{� )�1
���c  !9Rk�1Z�� )!�  {Z�1	���kBs�s�1 !���� �����)19)!c���B!!)Jk���{c1))RJ1!)1!BB�91� � �)�B� !)k�B� !))� )Z��� cc�)�� 9{�� !�)R��B  1Bk1�)B�{�  !�)Z�� �{  BB!�)B���B� ){1�)B�11 B��c  k))B��{) c{�c�  �)B�{�  !�)
���c  !9Rc�)R�� !�  {R�)	���kBs�k�) !���� �����)19)!c���B!!)Jk���{c1))RJ1!)1!BB�91� � ���B� )�����B�  R��� ������� scs���� �֥� ������B  9B{�����{�  !�ν� 	J��{  Bk�������B� ){�����11 B��c  ������{9 c��c� s�����{�  !��
���c  )9R����� ��k�  ����	���kBs�֌�<!�������� �9!1J919c��kkc)R���ZkkR)JJB11)1RscJ!c111�� �1�B� )11R� 	11J !1���  �  �1!� !!��  !�1��� �  �1B� )�1k�� � )1J��s�  �19� 1��1   � �1B� )1c�c�  �  )�1��� 1)!�  )�1����  �  �1!�������� �9!1J919c��kkc)R���ZkkR)JJB11)1RscJ!c111�� �)�B� !))J� 	!)B )���  �)� �  �)���  �)9� !�)Z�� !)B��s�  �)1� 
!��1    �  �)9� !)Z�c� �)���  �!�  !�)����  �)!�������� �9!1J919c��kkc)R���ZkkR)JJB11)1RscJ!c111�� ���B� ��� �� 
9���� ����� B9� ���k� 	�s1Zc!  9����� BB  !����� ��ν� c� k�����s�  ����� 9���1  R�c � ������ �����c� Z!�  {���� J���)  ������  )  ���K�������!)�!!)!�!J19RZs��Z��c{��{�cR1ZkB)1Rs���ƭ�kcZ9BJ�B1�  ��  �1 �� )�1 �� �1)!1�R  �!)� !�1{� !1!�� )11��B� )�  9�1�� !�1k�� � 1k�� �  �1�� �1� )k� �1�� !B�� !9�  �1�{� ))�  !11k�s� �  �1�������!)�!!)!�!J19RZs��Z��c{��{�cR1ZkB)1Rs���ƭ�kcZ9BJ�B1�  ��  �) �� �) �� 
!))!)�R� !� �)s� )�� !))��B� !�  1�) �� �)c��  � )c��  �){� �1� c� �) �� 9�� 1�  �)�{� !!�� ))c�s� �  �)�������!)�!!)!�!J19RZs��Z��c{��{�cR1ZkB)1Rs���ƭ�kcZ9BJ�B1�  ��  ��ƽƽ����ƽ��� ��� ���ƽ��ƽ�ƽ�ƽ��ƽ�����ƽ����ƽ����ƽ���� �� ���ƽ�� )J{����ƥ����R ����R� 	9���ƽƽ������  Z�� ��  Z����B  s��9  !����� )Bs����ƽν� !s�9� ����  s�{  !�ƽ���� 
B��1  )����� ���ƽ����ƽƽƽ�� 	)Bs������� )�����  s�� ����{� �����  Z��	��s  !���J�  ��� ��� ���ƽ��ƽ�ƽ�ƽ��ƽ�����ƽ����ƽ����ƽ���� ����ƽ����ƽ�ƽ��� ���ƽ����������!!9)!!)1))11!!�)	Bc)9Rk��kR��,��kZR{{ss����������Ƶ���ν{k)!9   �  B�1�R� )�1�� !)�1�  !11J1  �1�{� �1 �� 1���� 11�� !11kc� !)�1k�� !�� !k�� !){� )�1k�� � �� )1BR� !�1kc� 	!)B��!� )11k�  �1�{� !11k{� 11ν� )1��  !�1��������!!9)!!)1))11!!�)	Bc)9Rk��kR��,��kZR{{ss����������Ƶ���ν{k)!9   �  B�)�R� �)�� !�) �� ))B1� !�)�{�  �) �� ){��� ))�� ))cc� !�)c�� �� c�� !{� !�)c��  � �� !)9R� �)cc� !9��!� ))k�  �)�{� ))c{� ))ν� )��  �)��������!!9)!!)1))11!!�)	Bc)9Rk��kR��,��kZR{{ss����������Ƶ���ν{k)!9   �  B���ƽ����ƽƽ�������ƽ���ƽ���	��ƽƽ�ƽ����ƽ�ƽƽ�ƽ��� ��� ��� �������ƽƽ����������R� )�����  ����������  J���ƽ9  s�����ƽƽ����{� Z������ ������ s�����  ����c� ��������ƽƽν� ����� ��ֽ� Z����� �����Ό� ��9� s��� �����Z� ����������c� ��������!  �����k� R��ƽ�� �����{� !�����{  )����� ������ )�����ƽ���ƽ���	��ƽƽ�ƽ����ƽ�ƽƽ�ƽ��� ��� ��� �������ƽƽ����� ������ƽ�ƽ���� ���ƽ��ƽƽ�� ���� ���� ��!)!�!511JB91)J99RZJRcc��{c�����sJJ{���������ƜssccRZkssk{�c!�)  �  c�9 1�9 1�919919191�9191�9 1�9�1991�9 1�911991�9�191�911�9 1�919�19�{� )�9�{  )�911�9199�!  s99��  1�9 1�9191�9�{� 991�!� !�� 991�{� !1��� !�919�19911991k�� )1�1� R�� 99�� !991k�� 	!1  1B� !919�� !�919�1�9��� !1199��1� )199�� 1�919191�9k{� )911�� )k�B� 99�B� )191�9191�9 1�9�1991�9 1�911991�9�191�911�9 1�9 1�91911�9191�9�1	911919191���� ���� ��!)!�!511JB91)J99RZJRcc��{c�����sJJ{���������ƜssccRZkssk{�c!�)  �  c�)�{� !�)�{� �)�!� s))��  �)�{�  �)�!� �� �)�{� ){�� �)c�� )�1� J�� ))��  �)c�� )  )B� �)��  �){�� �){�1� !�) �� !�)c{� !�) �� !c�B� ))�B�  !�) ���� ���� ��!)!�!511JB91)J99RZJRcc��{c�����sJJ{���������ƜssccRZkssk{�c!�)  �  c���{� Z�����{  c���� ����!  B�����  ���� ����{� {����!� ����� B�����{� 	B��޽  9����ֽ� !�����1� c�ν� {����� c���ֽ� c���k!��B  ������ ����޽  9������1  ����� �� ���� ������{� )���� �� {���B  )�����B� c��� ����� �� �� �7!!!!!9JRRBB))BJRkss���sk{��{{BB�������ε��cc�s�R�BJ���J�1!�  �  s�9�R� �9�c  �9�� Z���  !�9�{� �9�B� !�!� 1�9k{� 1��  1�9k�� )99k{� 9s� 	9k�1  �9k�� �91!�� )�9�B� �9��  1�9��� !�9 �� !�9k�� 1�9�!� ��� Z���B�  �9� �� �� �7!!!!!9JRRBB))BJRkss���sk{��{{BB�������ε��cc�s�R�BJ���J�1!�  �  s�)�R�  �)�c�  �)�� Z���  �)�{�  �)�B� �!� !�)c{� )���  �)c�� ))c{� )k� )c�1�  �)c��  �)�� !�)�B�  �)��� !�)��� �) ��  �)c�� !�)�!� ��� R���B�  �)� �� �� �7!!!!!9JRRBB))BJRkss���sk{��{{BB�������ε��cc�s�R�BJ���J�1!�  �  s���R� �����c  ������ {���  �����{� �����B  ���!� ������{� 	���  Z����ֽ� ������{� s�ƽ  �����1  Z���ֽ� ��������� Z�����B� �����  Z������ )���� �� c���ֽ� 9�����!  9��� )�����B� 9���b�
��� ��B!))!))BcJ1BJBJ{s{����s��kZkZs�����Όk{���������cZcRBs�{�Z !� k�9�R� �9�!  �9RB� !�9�R� �9�B� )�� �9k�� )�c  �9k�� !�9R�� 99!J���� �9k�� �9ƌ� �9�c� �9�c  �9�{� 1�9�B� �9k�� 1�9kc� �c� ��{B� 1�9�
��� ��B!))!))BcJ1BJBJ{s{����s��kZkZs�����Όk{���������cZcRBs�{�Z !� k�1))1�)�1)1)1�)�1)1))11)�1)1�)�1)1)�1)1))1)11�)�1�)�1 )�1�)�1))11�R� ))11�!�  �1))�1JB� )1)�1�)11�R� 11)�B� �� )11)c�� �c� 
)1)1)11)11�)�1c�� )11J�� 
11  B����  �1c�� )�1��� ))1)�c�  �1�)11)1)1�c�  �1�{� !�1)�B� )�1	))11)11)c�� )�1cc� �c� ��{B�  �)�1)1))11)�1)1�)�1)1)�1)1))1)11�)�1�)�1 )�1�)�1 )�1)11)11)1)�1 )�1)11)�1�
��� ��B!))!))BcJ1BJBJ{s{����s��kZkZs�����Όk{���������cZcRBs�{�Z !� k���R� �����!  �����B� �����R  �����B  )���� ����ֽ� ���c  Z���ֽ� ����ν� ����{)������� c���ֽ� {����� J����c� �����c  Z����{� B�����B� J���ֽ� s�����c� ��c� s��{B� J����d�����	)� !�9�1;BZZBJskJ��{����{��JZkk������ss�������ƭ���s{sJJZJc��Z!� J�9�{� �9k�� �9 �� !�9�1  )�9�c� 1�� !�9��� !�B  �9{�� )�9Z�� �9{J{ޜ�� �9{�� �9�R� �9�{� �9�B  �9�{� 1�9�B�  �9k�� �9k�� {� �9�����	)� !�9�1;BZZBJskJ��{����{��JZkk������ss�������ƭ���s{sJJZJc��Z!� J�1�{�  �1c��  �1 ��  �1�1� �1�c� !�� �1��� �B�  �1s�� �1Z��  �1sBsޜ��  �1s��  �1�R� )�1�{�  �1�B�  �1{{� )�1�B� )�1c��  �1c��  s� )�1�����	)� !�9�1;BZZBJskJ��{����{��JZkk������ss�������ƭ���s{sJJZJc��Z!� J���{� ������� ����� B����1  B�����c  )���� ������ ���B  Z���ֽ� ������� �������޵�� B���ֽ� Z����R� Z����{� c����B  Z����{� Z�����B� )���ֽ� {���ֽ� ��� s������ ����� !�)@11cssk�����sZs�s��{���{{�sJks{�{���{s��������ƥ�{skkckRZZ99c��11� � �9�{� �9k�� 99R�9 � 1�9J�1  1�9�{� 1�!� )�9�{� �� 99R�9��� !�9k�� 
99���)  � 199��� �9{B� �9�{� �9 �� 99R99��� �9 �� 1�9��� �9k�� )1� ��9�� ����� !�)@11cssk�����sZs�s��{���{{�sJks{�{���{s��������ƥ�{skkckRZZ99c��11� �  9�1�{�  �1c�� 11J�1 �  )�19�1� )�1�{� )�!� �1�{� �� 11J�1��� �1c�� 	11���)  � )11���  �1sB�  �1�{�  �1 �� 11J11{��  �1 �� )�1{��  �1c�� !1� �1�� ����� !�)@11cssk�����sZs�s��{���{{�sJks{�{���{s��������ƥ�{skkckRZZ99c��11� �  9���{� ����ֽ� ������� c� �����1  s����{  )���!� �����{� s��� Z�������� ������� 
c������B  1� J������ Z����B� Z����{� Z��� �� Z������ޜ� Z��� �� ����޽� ����ֽ� c�1�  )�Z{��sc�����!�� �)!!�)BBcZs{�����9{c{����������{k{{�{��{c�sZJk���ֽ���{{kkJcZk�s1BJk�)!)� 1�9�s� �9J�� 9c�1�9�{99� !�9J�� �9�s� 1�!� )�9�{� �� 99�c�9�{� !�9k�� 9��c  � !� �9�s� �9 �� �9�R� �9 �� 99�c9J�� �9 �� )�9��� �9k�� �� �)1ks)1�9!�� �)!!�)BBcZs{�����9{c{����������{k{{�{��{c�sZJk���ֽ���{{kkJcZk�s1BJk�)!)�  �1�s�  �1B�� 1Z�)�1�s1)�  �1B��  �1�s� )�!� �1�{� �� 11�Z�1�{� �1c�� 
1��c  �  �1�s�  �1 ��  �1�R�  �1 �� 11�Z1B��  �1 �� !�1���  �1c�� �� �!)ck)�1!�� �)!!�)BBcZs{�����9{c{����������{k{{�{��{c�sZJk���ֽ���{{kkJcZk�s1BJk�)!)� 1���s� ���� �� Z����������ƭ{B� ���� �� �����s  9���!  �����{� ���� )��������{� ����ֽ� Z����c k���� �����s� ���� �� J����R� Z��� �� )������Ɯ� s��� �� 9����ޜ� ����ֽ� ���� �����ֽ����!� �� � ���I!))BBcc���������k{����{�������������sksBRZs��������������cckcRRJ9sc!� �9R�!� �9 �� 9�� �9Ό)�91)� !�9 �� �9�B� 1ks� )�9�{� !�1� 1k�)�9�{� !�9��� k�R  !1�B  !�k9��� !�9 ��  �9�B� �9�1� 1k�)9�� �9J�� 1�9k�� �9��� !�� )�9���!�9!� �� � ���I!))BBcc���������k{����{�������������sksBRZs��������������cckcRRJ9sc!� �1J�!�  �1 �� 1�� �1΄!�1)!�  �1 ��  �1�B� )ks� !�1�{� �1� )c�!)�1�{�  �1��� c�R  !�B� �k1��� �1 ��  �1�B�  �1�1� )k�!)��  �1B�� )�1c��  �1��� �� !�1����1!� �� � ���I!))BBcc���������k{����{�������������sksBRZs��������������cckcRRJ9sc!� ����!� ���� �� Z���� ���������)� ���� �� c����B  9���s  )�����{� ���1  )��������{� ����޽� Z���R  �����B  s������ ���� �� )����B� Z����1  )�������� ���� �� s���֜� s����� ���� )�������k���!��� �)!!)!)1BBZ����ƽ���{ k��%�����������������sk{sc�{s{{����������{�ZsR9)JB� � B99B99BB�9B99B�9B99B�9 B�9BB�9 B�9B99B9B�9BB�9 B�9BB�9BB9B�9B9B9B�9BB9{�� !�9�B  9�{ )�9B9�B!!99B99JR  !99B�9 B�9BJ�� B�9�{� 19�� 19B9J�s� 1�{� 1�� !B�9B9B9B9�{� !�9��� �� !9��� )� B�{� )B9BB99�� 1�9�1� B99B99B99B�9�{� 1�� !cB� !BB99��� �9B99B99B9B9k��  �9�s� )�� 199BJ��  !9B99B�9 B�9BB�9 B�9B99B9B�9BB�9 B�9BB�9BB9B�9B9B9B�9B99B�9 B�9BB!��� �)!!)!)1BBZ����ƽ���{ k��%�����������������sk{sc�{s{{����������{�ZsR9)JB� � �1s��  �1�B� 1�{ �1�B!�1BR  �1B��  �1�{� )1��  )�1B�s� !�{� )�� �1�{�  �1��� �� 1��� � 9�{� �1 �� )�1�1�  �1�{� )�� ZB�  �1��� �1k��  �1�s� �� )�1B��  )�1!��� �)!!)!)1BBZ����ƽ���{ k��%�����������������sk{sc�{s{{����������{�ZsR9)JB� � ����� �����B  Z���{ �����B)���{  ���� �� Z����{  )��Ƶ� �����s� c���{  )��� k����{� c����� 9���  ������ Z�� ��{� ���� �� )�����1� Z����{  )��� k��B  9������ ����ֽ� 9����s  9���� )������ ����� � �� � ���;!!191)9Jc{��ν��������������������������������{ssk{���������έ�ZRRZJJRB!� � �BZƭ� !�B�B� J�B !�B�B 1�B��  )�BZ��� �Bc�R� 1BJB  !�B��� Bs{� 1�{ �B�B�  �B�{� �!� B���� R JB� )�B{!� 1BBJ�� �Bs{� 1�{ Bk� 9BBc�{� )�Bs�� 9BBs�!  9Bc� 1BBJ�� )�B�� � �� � ���;!!191)9Jc{��ν��������������������������������{ssk{���������έ�ZRRZJJRB!� � �1Rƭ�  �1�B� B�B  �1�B  )�1��  �1R���  �1Z�R� )1BB�  �1��� 1k{� )�{ �1�B�  �1�{� �!� 9���� R B9� �1s!� )11B��  �1k{� )�{ 1k� )11Z�{� !�1k�� )11k�!  )1c� )11B�� !�1�� � �� � ���;!!191)9Jc{��ν��������������������������������{ssk{���������έ�ZRRZJJRB!� � ����� J����B  9���B �����B B����� !�������� Z�����R  ����J  B�����  !����{  ���{ Z����B� !����{� 9��!  )������ J�Z ��� �����!� ���� �� Z����{  ���{ Z�Ƶ�  �����{� {����ֽ� �����!  ����k  ������ s��������� �*!!)!!9RJBJc{��������������Ƶ��������
���Ƶ�ƥ����k s�k {����Ƶ�ƽ�{�c{{Z!� �Bs���R� s�BZ{� �� !�B�c  9BJ��{ 1�Bc��c� BBZ��� )BBJ!  ����c� 1BB�� ��9 �BZ�1� 1B��B� �� ��B� )� )�B J� )B����  �B �� 	��9 BBR� 
B֭��  �Bs�� 9s���� !BBc  c����) 9�B���� �*!!)!!9RJBJc{��������������Ƶ��������
���Ƶ�ƥ����k s�k {����Ƶ�ƽ�{�c{{Z!� �911�9�1�91911�911�911�91991�911�9�1�9�1�9 1�9�19191�91919911919191k���R� s119R{� 	�� 991�9�c  )1B��{ )9911�9191Z��c� 19R��� !919!� ����c� )19�� {�9  991911�9R�1� )1��B� �� ��B� � !�  !�91991J� !9���� 191991�91919�� 	{�9  99R� 9έ��� 1919191�91991k�� )k���� 11Z� Z����) 19911�911�91991�911�9�1�9�1�9 1�9�19191�91919911919199199191�911�91911�9119919���� �*!!)!!9RJBJc{��������������Ƶ��������
���Ƶ�ƥ����k s�k {����Ƶ�ƽ�{�c{{Z!� ������R� �����{  ��� �����c  {����{ R�������c� )������� !��ƽ)  c����c  ����ƽ� ���9 {�����1� {����B� B��� ���c� �!�J� ���� �� ������� ��� �� ���9 {��ƌ  �����  Z����ֽ� Z�����  9�����  Z�����1 )�����!��  ��!�)!)9BccZk�����������ε��ν���������޽������scc�k{{s{��������{��Rc��c!� �Bs�)� )�B �� J )�B� 	9��9  9�Bs�� 1Bc�� 
)BB9  ZR1� )�Bs!� 11  �B��� BZc� s� � )�  �BJ  !��1� �Bs!� 11  �B 1� JBZ  �BJ�� �{� 9�B Z� 	{{s  1�B!��  ��!�)!)9BccZk�����������ε��ν���������޽������scc�k{{s{��������{��Rc��c!� �9k�)� !�9 �� B �9� 	1��1  1�9k�� )9Z�� 
!991  ZR1� !�9k!� 11  �9��� 9Rc�  k� �  !�  �9 B� ��1� �9k!� 11  �9 1� JBZ� 1�9B{� �{� 1�9 R� 	{{s  )�9!��  ��!�)!)9BccZk�����������ε��ν���������޽������scc�k{{s{��������{��Rc��c!� ��ֵ)� !���� )c 9���� s� {���9 ������� 9����� ��Ƶ!  RZZ9  �����
�!  k1 ������� ��΄� {�� ZZ� {�� Z��� J���Z�  ��
�!  k1 ����{  !kBk  B����	�   ��{� ���� �� 	R{{s ������ �!�� �� ���!!))119�1RZck�������������������.�����Ƶ�ƭ����������{scJRZR{����{cRJ��sR��R)� �B1�  � !�B J� 9�B 1�s )�B�  9B1!�  )�B!�  !�B Z� )�BJ)� 1B9� 1� )� � � !9�B1B))� )�B Z� )�B9� �9�B91� � !�B1� )�B �!�� �� ���!!))119�1RZck�������������������.�����Ƶ�ƭ����������{scJRZR{����{cRJ��sR��R)� �9)� �9 J� 1�9 )�k � �9� )9)� !�9 � �9 Z� !�9B!� )91� )� !� �� 1�9) 1!!� !�9 Z� !�91� 1�91 )� � �9)� �9 �!�� �� ���!!))119�1RZck�������������������.�����Ƶ�ƭ����������{scJRZR{����{cRJ��sR��R)� ���1 J!�)���� k� ����������) !����� �� ){J9��Ɯc9ZZB �����{� )9����� �� s���� B� ��ƭJ )s�{� B����  )�{1)� !9���	�9{��{9�{� ���� �� s������9� ))Z�����R��cZ�Z  c������B� �����|!!!��� �!!�)�19Jcs{����������ƽ�������������ν��������ssc�RZRZcs��sRBc�����R!� �B
!!!!!!)�B! �!�B99�B1!1�B9)1!1�B)!))!)�B91!!)�B) �B)1�B1!�B9�1�B !�	9B!!1�B)!�B	!)9))�B) �B9)!!9�B1)B9)1!9�B1!�!�B!!!��� �!!�)�19Jcs{����������ƽ�������������ν��������ssc�RZRZcs��sRBc�����R!� �9� !�9 � �911�9))�91 !))�9!!�91) !�9 )� �91! � !�9)91  )�9 �)9�)�9!�9!)� !�9 )� �9)! �1�9)!9)!) 1�9!� �9!!!��� �!!�)�19Jcs{����������ƽ�������������ν��������ssc�RZRZcs��sRBc�����R!� �������������k)Bss�������������������������������s�������99sc�������{��{�����������ƵRc������� ����ƽ�����������	��ƽ�cZ������99sc������c�������
���ƽ�9B��������ZBs�����0)!!!���  ��!!1!!19R�s{�������������������� ��������$ֽ��Ƶ���{s��cckcs{s����Z1!R�{BsB  � �B9!!119�B19�B99�B 9�B91�B)!)11�B91BB91�B1)19�B1)1�B)!)11�B11�B1!1�B9!)9�B)!!!���  ��!!1!!19R�s{�������������������� ��������$ֽ��Ƶ���{s��cckcs{s����Z1!R�{BsB  � �91!!1�9)1�9))�9 )�9)!1�9!))�91)11))�9))1�9 1�9)!!�9))�91))�9))�91!1�9)!!!���  ��!!1!!19R�s{�������������������� ��������$ֽ��Ƶ���{s��cckcs{s����Z1!R�{BsB  � ������ ��� ������� ��� ���������������������������M�!!� � ���!!)!)Bck�sk{������������������ ��� ���$�����Ɯ������k{���s���������Z!B�{RZ� �B�B�B�B�!!� � ���!!)!)Bck�sk{������������������ ��� ���$�����Ɯ������k{���s���������Z!B�{RZ� �9�9�9�9�!!� � ���!!)!)Bck�sk{������������������ ��� ���$�����Ɯ������k{���s���������Z!B�{RZ� �ƁƁƔ�% )�!��  � �)9Z9!9RZcs����������������������� ����� ���	���ƽ������� s�{�����ƭs9!Z�sR!� �J B�J B�J B�J B�JBJBB�J B�J B�J B�JBB�J B�J B�JBB�J B�J B�JBJB�JBJJB�J B�J B�J B�J B�J B�J B�JBB�JBJJB�JBB�JBB�JBJBJJB�J B�JBJB�JBBJJB�J B�J B�J B�J B�J B�JBJBB�J B�J B�J B�JBB�J B�JBJBJB�J B�JBJJ)�!��  � �)9Z9!9RZcs����������������������� ����� ���	���ƽ������� s�{�����ƭs9!Z�sR!� �9�9�9�9 )�!��  � �)9Z9!9RZcs����������������������� ����� ���	���ƽ������� s�{�����ƭs9!Z�sR!� �ƁƁƔ� 1))�! ����  �3 !1Z�{1!1BZc����ƽ����������������������������������
������ƥ�����{{kk�������Ό)R�k)� �J�J�J�J1))�! ����  �3 !1Z�{1!1BZc����ƽ����������������������������������
������ƥ�����{{kk�������Ό)R�k)�  9�B9B9B9BB9�B9B9B9�B�9B9�B�9BB9B9B9BB99B99B9�B9B99�B 9�B 9�B9B9BB99�B9BB9�B99BB9�B9B�9�B 9�B99B�9�B 9�B�9B99�B�9�B99B99B99�B	9B99B99B99�B99B9B9B�9�B9B�9B99B9�B�9 B�9�B
9B9BB9B9B99�B99B99�B 9�B99B9�B9BB9�B9BB9�B 9�B 9�B 9�B 9�B99B9�B 9�B 9�B�9B9BB9B9BB�9BB9B9BB9�B9B9B9BB9�B9B9B9�B�9B9�B�9BB9B9B9BB99B99B9�B9B99�B 9�B 9�B9B9BB99�B9BB9�B99BB9�B9BB99B99B�9�B 9�B99BB9B�9B1))�! ����  �3 !1Z�{1!1BZc����ƽ����������������������������������
������ƥ�����{{kk�������Ό)R�k)� �ƁƁƔ�k!11)!!�� �� �  9s��1)9c���
������������� ��� ��������������ƭ������ss�������Ό1!JsJ� �JRsR�J�J�J�J!11)!!�� �� �  9s��1)9c���
������������� ��� ��������������ƭ������ss�������Ό1!JsJ� �BJsJ�B�B�B�B!11)!!�� �� �  9s��1)9c���
������������� ��� ��������������ƭ������ss�������Ό1!JsJ� ����΁ƁƁ���N�)!!�� �� �� �!k�� !Bs��������������&���������ֵ�޽��������{���������{)Jc)� �J����������ֵ{���������c�Jk���R�J�J ��� {�J�Jk���Ɯ{{kJcƽkJJk������R�J�����Jk{����R�Jk{����R�J�)!!�� �� �� �!k�� !Bs��������������&���������ֵ�޽��������{���������{)Jc)� �B���{������֭s���������Z�Bc���J�B�B ��� s�B�Bc���ƔsscBZƽcBBc������J�B�����Bcs����J�Bcs����J�B�)!!�� �� �� �!k�� !Bs��������������&���������ֵ�޽��������{���������{)Jc)� ��������
��������������������΁��� ��� ց�����������	������������������������������������������))!!���   )J���! J��{����������� ����������	������������$��������ֵ�ν��ε������������ֽ�19�R1� �Jk��c��s{{�{B9Z�c!  )B1BBB�J��R)B�J�JR�����{ ��JR�k�J�������J��B R������R99c�Ό��{c9B�Jk�{19�J�ν�B )B�J�ν�B )�J))!!���   )J���! J��{����������� ����������	������������$��������ֵ�ν��ε������������ֽ�19�R1� �Bc��c��s{{�{B9Z�c!  !B1BB9�B��R)9�B�BJ�����{ ��BJ�c�B�������B{�B R������R91Z�Ό��{c99�Bc�{19�B�ν�B )9�B�ν�B )9�B))!!���   )J���! J��{����������� ����������	������������$��������ֵ�ν��ε������������ֽ�19�R1� �����c��s{{�{B9k�c!  )B1BB�����R9�����������{ �����������������B R������RR���Ό��{cR�����{1����ν�B 9����ν�B 9���[ )�!��� � )Bc��1 J{R{���ƽ������������������������ ������ƽ�ƥ�Ƶ��������ƭƽ��kssB� �J ��  � 1�J�R� 9�J�J��s!� B�J��c9�J���BB! 1�J 1� 1J� 9�J{��  �J{� 9�J{� 9�J )�!��� � )Bc��1 J{R{���ƽ������������������������ ������ƽ�ƥ�Ƶ��������ƭƽ��kssB� �B ��  � !�B�R�  )�B�B��s!� 9�B��c1�B���BB! 1�B )� )B� 1�Bs��  �Bs�  )�Bs�  )�B )�!��� � )Bc��1 J{R{���ƽ������������������������ ������ƽ�ƥ�Ƶ��������ƭƽ��kssB� ��� 1� {����R�  B������s!�  J��������BB! R�� �� k���  Z�����  �����  Z����  Z��)�!��� � !9RRcJ9kc{��������������� ���������&�����������ֽ��ƥ��Ƶ�������ν�ƥ{k{�k!� �J� � !�J�B� B�JJ{�R� B�J�c 1�JkJ� B�J �  JJ!� �JZ�� )�J9� �J9� �J)�!��� � !9RRcJ9kc{��������������� ���������&�����������ֽ��ƥ��Ƶ�������ν�ƥ{k{�k!� �B� � �B�B� 9�BBs�R� 9�B�c )�BkJ� 9�B � BB� �BR�� !�B1�  �B1�  �B)�!��� � !9RRcJ9kc{��������������� ���������&�����������ֽ��ƥ��Ƶ�������ν�ƥ{k{�k!� �����������9)� )s���!� Z������������B� B��� �������� ��� ��� ��� ��� ��� ��� ��� ����������������R�  ��� ����������c R����������Z� s��������� ��� ��� ��� ��� ����!� J9c��ƽB� ����ν� ������Ɣ)� Z�������)� Z��� ��� ��� ���������!!)���!)BB9ZJRB9JZ�����������������������������޵�絵ƽ���������νƜJJ��Z� RJRJJRRJRJRRJR�JRRJJRJJ�R�J�R91� )JJ��  �JRR�JRJR�JRRZc� JJRJRJJRJJR�JRJRRJ�RJRJJR�JRJ�R J�RJRJ�R�JRRJ�RJJRRJR�J�RJJ�RJRRJ�RJJ�RJJRJJ�R�J R�JRRJ�RJRJR�J���  � 1JJRJ�RJRJRJRRJJRRJRJ�R�JRJRRJJ�� JRRJJRJR�JRJRRJB� R�J R�JRJJRR�J�RJRR�J�R�JRRJR�JRR�JRJRJRJR�JRRJJRJJRRJRJRRJc1� !)!JRRZ)� )JJRR�J R�JR�� )JJ�R�JRJR�J�� RR�JRJRJJRJJRJ�� 	RJRJJRJJ�RJRJRJRJJRJJRJJRJRJRJR�JR!!)���!)BB9ZJRB9JZ�����������������������������޵�絵ƽ���������νƜJJ��Z� �B1)� !9B�� �9�BRc� 9�BB��� !�B ��  �B9�  �BZ1� !9BBR)� !�B �� !�B��  �B��  �B!!)���!)BB9ZJRB9JZ�����������������������������޵�絵ƽ���������νƜJJ��Z� ������������������������������ 9������  J����������������k� ���������� ������������� ������������������������ ������������������������������������ ��������� J� ��������������������������������  )�������� ����1� Z������ �������������������������������������	���������������� Z� �������9� ����������������� )��������������� Z�������������� Z�����������������������������}!!!���  �!)B11B1BJ)1R������������������������'�������������ν�ƭ�������ƭ���ƭ΄c��)� �R{{� B�Rc9  !9�RJ !�RR{�c�   1�R��  B�R� �R �� 9�R �� )�R)  B�R�{� �R�{� �R!!!���  �!)B11B1BJ)1R������������������������'�������������ν�ƭ�������ƭ���ƭ΄c��)� BJJBJBJBBJBJB�JBBJJBJJBJBBJBJJBB�JBBs{� 9JBBZ1� 1BBJJBJBJJBJBJJB�JB  JBJJBJB�JBJB�J B�JBJJBBJJB�J�B�J B�JBJBBJJB�J B�JBJJBJJBJ�BJJB�J B�JBB�J B�JBBJ�B�JBJJ�BJBs�c� � )BB�J B�J�B�J B�J�BJBJ�BJJ��� 9B�J	BBJBBJBJJB�JBR� 
JBBJBJJBBJ�B�J B�J B�JBJBJBBJBJ�BJBB�JBBJJBJB�JBJJBJBJBBJBJBJ�� 1JBJBBJ�� !�B J�BJJBJBBJB)  9BBJJBJBJJBJBJJ�{� 
JJBJBJJBJB�JBJ�{� BB�JBJBBJB�J B�J�BJB�JBB!!!���  �!)B11B1BJ)1R������������������������'�������������ν�ƭ�������ƭ���ƭ΄c��)� �� ��� ��� ��� ����������{� Z����	k ��������� ���� s���� ��� ��� �������� ��� ������� ��� ��� ��� ��� ��� �����c� 	J�s J������ ��� ��� �����  B����������������Ɯ� Z��� ��� ��� ��� ��� ��� ���������������� ��� �� B���� �� s����� ���s !�������� ����{� Z������ �����{� Z��� ��� ��� ����!!���    � �)9)!B!)B{��������������������������������'���������������޽��������ֽ��ޭƵ))J�Z!� �R{�� �RBB)�R1!J�RZ{{Z�Rk{��s�Rk{{�Rss�RZ�� JJ!J�Rs��� ss�R�B� �R�B� J�R�B� �RJ9�R �� �R �� �R!!���    � �)9)!B!)B{��������������������������������'���������������޽��������ֽ��ޭƵ))J�Z!� �Js��  �JB9!�J)B�JRssR�Jcs��k�Jcss�Jkk�JR�� BBB�Jk��� kk�J�B�  �J�B� B�J�B� �JB1�J ��  �J ��  �J!!���    � �)9)!B!)B{��������������������������������'���������������޽��������ֽ��ޭƵ))J�Z!� ��ֽ� �����������Ƶ������������������������� Z��ƽ��������  )������B� Z����B� c�����B� ��������� �� {��� �� {���D!!���� ��!!!1 !!9{{Zk���������������� ��� ������������������ƭ�����޽��֥�cB)�J9� �R��� )RR���R B�R����޽�R��ｭZR��޽�{�k�RZ��޽��kRZ��ތ�Rs���ε��s�R���!� ��R��� J���R�{� ����֔�R����֭{�R�B� �R�!� 1�RZέ���{�R �� !�R �� !�R!!���� ��!!!1 !!9{{Zk���������������� ��� ������������������ƭ�����޽��֥�cB)�J9� �J��� JJ���J 1�J����޽�J��ｭRJ��޽�{�c�JR��޽��cJR��ބ�Jk���ε��k�J���!� ��J��� J���J�{� ����֌�J����֥s�J�B�  �J�!� )�JRƥ���s�J ��  �J ��  �J!!���� ��!!!1 !!9{{Zk���������������� ��� ������������������ƭ�����޽��֥�cB)�J9� ��޽� �������� �������������ｽ����޽�{������޽�������������������������!� 9������� Z�����{� �������������������B� Z����!� ������������ �� ���� �� ����I!�� � �!)!!1J�c9Z��������������������������������������Ƶ����������	���9{!1� �R��� 1R��s)�R��c{B1!�R�� Z��� B�R���{� B���R1�R
���c  !9R{�R���  !�R��B�  �R�{� R�c  BJ�R
Z���Z 1B{�s�R�B�  �R�B� 1�Rk�R{c1)9�R�!  !�R�!  !�R!�� � �!)!!1J�c9Z��������������������������������������Ƶ����������	���9{!1� �J��� )J��s!�J��c{B1�J�� R��� 9�J���{� B���R1�J
���c  !9Rs�J���  !�J��B� B�J�{� B�c  BB�J
R���R 1B{�k�J�B�  �J�B� )�Jc�R{c1)9�J�!�  �J�!�  �J!�� � �!)!!1J�c9Z��������������������������������������Ƶ����������	���9{!1� ���� �����sZ���c{B1J���� �ޥ� ������{� J���R1���
���c  )9R���ޜ�  Z����B�  R���{� {��c  B���	���Z 1B{�����B� 9����B� ������R{c19����!  {����!  {���8 )�� � �)19911)1c9!R��������������	��������������޽������ ��������Υ�Zs !� �R�{� 1��1  !J�R B� !B�R1� )1�  1�Rs��� !�  !�R��c�  �  1�R 9� J�R Z� J�R�{�  � !J�R��c�  � !B�R�s� J�Rk�� 1�RJ� )�R�1  !�R�1  !�R )�� � �)19911)1c9!R��������������	��������������޽������ ��������Υ�Zs !� �J{{� )��1  B�J 9� 9�J1� !1�  )�Jk��� !�  �J{�c�  1�J 1� B�J Z� B�J�{�  � B�J��c� !9�J�s� 9�Jc�� )�JB� !�J�1�  �J�1�  �J )�� � �)19911)1c9!R��������������	��������������޽������ ��������Υ�Zs !� ���{� ����1  ���� k� ����s� 
�{1Zc!  9������� )�  �����c� Z!�  {�� {� ))c���� �� 9�����{� 9R� ������c  )� 1����s� !������� ������ Z����1  Z����1  Z���H!!� �  �)9BcZZR)B)1k�������������&�����������ֽ�֭��ε�����������ޥ��Z1  �� �R��� ��{� 1�R �� 1�R�� )R))�� J�R��� � )�Rc�� )9�  !�R{� !!�R �� J�R�{� �  )�R	��1  )�  1�R��� �������� 1�Rs!�  �R �� �R �� �R!!� �  �)9BcZZR)B)1k�������������&�����������ֽ�֭��ε�����������ޥ��Z1  �� �J{�� {�{�  !�J ��  )�J�� !J!��  B�J���  �  �JZ�� !1�  �Js� �J �� B�J�{� �  �J��1� !�  )�J{�� �������� )�Jk!�  �J ��  �J ��  �J!!� �  �)9BcZZR)B)1k�������������&�����������ֽ�֭��ε�����������ޥ��Z1  �� ��ֽ� ���{� {��� �� ���� � Z�ν��� Z������ s�c� ������� )�����  s�� !� s����� �� )J{������{� 9{{!� !�����1  B���!  ���֭� ��������� �����1� )��� �� Z��� �� Z���� !����1cZB)!)9)!)11{������������ �������+���������������ƽ�Ƶ�����������ֵ��{9   �  Z�R Z�RZRRZ�RZc�� � !�R Z�R�R� !�R Z�RZ�{� RZR�� RZ�R�s� !R{� !�R Z�R Z�R Z�Rc��!� 9ZRk�  �RZRRZ�R��� �RZRZ�RZRR�� !)9J�R Z�RZR�{� B�� B�RZRRZRRc�{� JR�� 9�R Z�RZRRZ�R Z�R��� Rcc�{ !� 1RRZ�R Z�R �� B�R Z�R�1  �R Z�R�1  �RZRRZRRZ�R !����1cZB)!)9)!)11{������������ �������+���������������ƽ�Ƶ�����������ֵ��{9   � �JZ�� �  �J{R�  �J�{�  �J ��  �J�s� Js� �JZ��!� 1JJk�  �J{�� �J�� 1B�J�{� 1�� 9�JZ�{� BJ��  1�J��� Jcc�{ !� )�J �� 9�J�1�  �J�1�  �J !����1cZB)!)9)!)11{������������ �������+���������������ƽ�Ƶ�����������ֵ��{9   � �� �� 9� J����R� Z����{� Z��� ��  ����s� ��έ� ������!  �����k�  R��֭� ����
�  ���������{� 9��Ƶ� ����	�{  ���έ� Z���޽� Zkc�{ !� ���� �� �����1  Z����1  Z���D!��  1{s1�!)k�sks���������� ��� ���+����������������������ƽ�������������c�c� � �Z �� R�Zs�� �Z�{� �Z�!� 1�Z�!� 1Z�c� 1�Z��1� 9�Z �� J�Z��� 9�Z�{  9�Z�{� JZ�� 1�Z
��  1Zc�� R�ZΜ� � 1�Z�B� 1�Z�1  �Z�1  �Z!��  1{s1�!)k�sks���������� ��� ���+����������������������ƽ�������������c�c� � JJR�J�RJRRJRJRJ�RJR��  �JRRJ�R J�RJRs�� JRJ�RJJ�RJR�{� JJR�!� )J�RJRRJJRR�!� )R�c� )RRJRJR�JRR�J�R J�R��1� 1RRJ�� BJRJ�R{�� 1JJRRJ�RJJRJJ�RJRJ�RJRRJJ�RJJ�RJRRJJ�{� 1R�JRR�JRRJJ�{� BR�� )JJ�RJRJRJJ��� )RZ�� J�RJJRRJJRJJ�RJRJJR�J�RΜ�  � )RRJRRJ�R J�RJ�B� )J�R J�R�1� RRJJRJ�RJJ�R�1�  �R J�R�J�RJJRRJJ�R�J�RJRR�J!��  1{s1�!)k�sks���������� ��� ���+����������������������ƽ�������������c�c� � �� �� !���ֽ� Z����{� {����!� �����!� ����c� 9JB�������1  ����� �� ����ֽ� )�����{  c�����{� B������ ����
��  Z������  ����� )!� �����B� �����1  J����1  J���{!!� �   Z�9�1JRc��������������������'��������������������ƽ�������������scsZ� � �Z �� B�Z��� �Z�{� !�Z�B� 1�Z �� 1Z�s� 1)9�Z��� )�Z �� )�Zk�� J�Z�c  �Z�{� JZZ�B� )�Z�{� ����� )�Z�R�  ��� 1�Z�R� 1�Z �� R�Z �� R�Z!!� �   Z�9�1JRc��������������������'��������������������ƽ�������������scsZ� � �R ��  9�R{��  �R�{�  �R�B� )�R �� )R�s� )1�R��� !�R ��  !�Rc�� 9�R�c�  �R�{� 9RR�B� !�R�{� �����  !�R�R� �� )�R�R� )�R �� J�R �� J�R!!� �   Z�9�1JRc��������������������'��������������������ƽ�������������scsZ� � �� ��  !� s������ �����{� �����B  ���� �� ����s� ��������� )���� �� c���ֽ� )�����c  �����{  ������B� �����{� Z������ c����R� {�������� �����R  )���� �� Z��� �� Z�����!� �  � Z�B�!9 !1RcZ��cRs����������� ������������������������ƽ���������ν�Ƶs1k9� � �Z �� � R�Z��� !�Z�R� !�Z�B� 9�Z	�  BZ�B� 1�Z�{� J�Z�B� �Z �� J�Z�!  !�Z�{� B�Z�{� )�Z��!� ��{!� �Z�B� )R�ZRJ�!� 9�Z�B� J�Z �� �Z �� �Z�!� �  � Z�B�!9 !1RcZ��cRs����������� ������������������������ƽ���������ν�Ƶs1k9� � �R �� � J�R{��  �R�R� �R�B� 1�R�� 1R�B� )�R�{� B�R�B�  �R �� 9�R�!�  �R�{� 1�R�{� !�R{�!� ��{!�  �R�B� !J�RJB�!� )�R�B� B�R ��  �R ��  �R�!� �  � Z�B�!9 !1RcZ��cRs����������� ������������������������ƽ���������ν�Ƶs1k9� � �� �� !�s�  Z��޽� c����R  �����B  )�����  J����B  �����{� B�����B� J��� �� 9�����!  �����{  )�����{� ������!� !{��{!� {����B� )������!� �����B  ���� �� Z��� �� Z����!!�   �)sJ  �)J )BZ91R)�J������������������ν���������������������������������絭���ƔJ1B!� � �Z�1� B�� J�Z��� �Z�1  9�Z�c� J�Z{s  RZ�B  9�Z�{� R�Z�B� R�Z �� J�Z��� !�Z�{� J�Z��� 9�Zs�� !�Z�{� J�Z�B� 9�Z�B� 9�Z�1  �Z�1  �Z!!�   �)sJ  �)J )BZ91R)�J������������������ν���������������������������������絭���ƔJ1B!� � �R�1� 1�� B�R{��  �R�1� 1�R�c� B�Rss� JR�B� 1�R�{� J�R�B� J�R �� B�R{��  �R�{� B�R��� )�Rk�� �R�{� B�R�B� )�R�B� 1�R�1�  �R�1�  �R!!�   �)sJ  �)J )BZ91R)�J������������������ν���������������������������������絭���ƔJ1B!� � ���1� ��Ɣ�  s��޽� Z����1  B�����c  )�����s  B����B  B�����{� Z����B� )��� �� Z������� �����{  !������ ����ֽ� �����{� B�����B  !�����B� �����1  Z����1  Z����� �� �
 JB�  )R !JR  !91c�������������������޽�c9B1)Bs�Υ��������������������������������έ��{��9J!�  � �Z�B� 9Z�)� �Z{�� �Zc�1  R�Z�{� J�Z �� 	c��!  R�Z��� �Z �� J�Z �� �Z��� !ZZs�Z�s� B�Z��� 9�Z�� ��1�Z�{� R�Z�B� B�Z�B� 9�Zs�  �Zs�  �Z� �� �
 JB�  )R !JR  !91c�������������������޽�c9B1)Bs�Υ��������������������������������έ��{��9J!�  � �R�B� )R�)�  �Rs��  �RZ�1� J�R�{� B�R �� Z��!� J�R���  �R �� B�R ��  �R��� RRk�R�s� 9�R���  )�R�� !�R�{� J�R�B� 9�R�B� )�Rk��  �Rk��  �R� �� �
 JB�  )R !JR  !91c�������������������޽�c9B1)Bs�Υ��������������������������������έ��{��9J!�  � ���B� Z����)� B���ֽ� �����1  s����{  )����� ����! ����ޜ� Z��� �� ���� �� Z���޽� ��������s  !������ ���� �  B�Z��c{�����{� B����B  )�����B  )������  Z�����  Z�����  �)B�)!1!!JR 9!� 1Z�{k�����������������������{s{JB11ZR9B1Jk��������������������ƽ�������罥�sk�c9!1� �Z�!� �Z R� R�Zc�� !�Zk�� �Z�s� J�Z��� B�)  1�Zk�� �Z �� B�Z �� �Zk�� !Z{�R�Z�B� B�Z�{� )�Z�s� �9BR�c9�Z�{� R�Z�B� J�Z�R� J�Z{�� �Z{�� �Z�  �)B�)!1!!JR 9!� 1Z�{k�����������������������{s{JB11ZR9B1Jk��������������������ƽ�������罥�sk�c9!1� �R�!�  �R� J�Rc��  �Rc��  �R�s� B�R��� 9�)� !�Rc��  �R �� 9�R ��  �Rc�� Rs�J�R�B� 9�R�{�  !�R�s� �)9J�Z)�R�{� J�R�B� B�R�R� B�Rs��  �Rs��  �R�  �)B�)!1!!JR 9!� 1Z�{k�����������������������{s{JB11ZR9B1Jk��������������������ƽ�������罥�sk�c9!1� ���!� {��� ��  ���� ���� �� �����s  9������� ���9  {���� �� s��� �� 9���� �� Z��� �� Z��������B  !�����{� �����s� 9�����������{� )����B  )�����R  )������� Z������ Z����!!!�  � )  )J1J)JR  !B� 	J��������&�����������������ޭ��Ό{9!Rc{�{J��������������������ν��������޵��kk�J!)� �Z�B� !�Z ��  1�Z��� )�Z �� !�Z�B� R�Zs��  !9!�Z �� �Zk�� R�Zk�� �Z �� Z�� �Z�B� J�Z�{� )�Z�{� R�Z��cB�Z�{� J�Z��� J�Z�s� J�Z ��  �Z ��  �Z!!!�  � )  )J1J)JR  !B� 	J��������&�����������������ޭ��Ό{9!Rc{�{J��������������������ν��������޵��kk�J!)� �R�B�  �R ��  !�R���  �R ��  �R�B� J�Rk��� 1�R ��  �Rc�� J�Rc��  �R �� R�� �R�B� 9�R�{�  �R�{� J�R��cB�R�{� B�R��� B�R�s� B�R ��  �R ��  �R!!!�  � )  )J1J)JR  !B� 	J��������&�����������������ޭ��Ό{9!Rc{�{J��������������������ν��������޵��kk�J!)� ���B� ���� �� 9������ J��� �� c����B  9������  B���1R����� �� ���� �� s��� �� Z��� �� Z���� ���B� �����{� �����{� Z�����s����{� )������� �����s� ����� )���� )����!!)�  � 1� T1JB!9J  )1  BJBZ������������������������������޵�Zk�����޽�����������������������������ƽ��{scR1!!�  Z�c Z�cZc�ZcZcc�ZcZ�cZZcck�� )Z�c�1� RcZcc�Z�c Z�cZ��� Z�c�ZcZccZcZk�� cZcc�{� RZ�cZccZc�� JcJ1)JZ�c Z�cZcZZccZ�c�ZccZ�cZccZ�cZZc{B� )Z�c��� ZZccZ�c�Zcc��� cZ�cZZ�c Z�cZcZZcZ�c Z�cZ�B  Z�{ !9�cZccZ�c�Z�B� 9cZc�c� )�c Z�c Z�cZ�� Z�c��J 1�Z�cZZcZcZZ�cZcZ�cZZ�c Z�c��B� Jcc�Z��� 9Z�c Z�c Z�ck�!� 1cZcc�Z�cZcck�� JcZZ�c�ZcZccZk�� JZ�c Z�cZcZZ�cZccZZ�cZc!!)�  � 1� T1JB!9J  )1  BJBZ������������������������������޵�Zk�����޽�����������������������������ƽ��{scR1!!� RRZ�RZRZRZZR�Z R�Z�RZZ�R Z�RZRc�� ZZRR�1� JZRZZ�R
ZZRZZRZRR��� RZR�ZRR�ZRZRc�� RZRZ�{� JRRZ�R�Z�� BZB)BZZRZ�R�Z�R Z�RZR�ZRZ�RZZ�RZRRZZRsB�  �RZ��� ZZ�R�Z�R���  �R�Z R�Z�R�ZRZRZZRR�Z R�Z�RZZ�B� R�{ 1�Z�RZZRZRRZ�B� 1RZZ�c� ZRZ�RZZRZZRZ�� ZRZR��J )ZZRZRRZRZRZZRZZRR�ZRR�ZRZ�R	ZRZRZZR��B� BZ�RZ��� 1RZRZZ�R	ZZRZZRRc�!� )ZRZZ�RZZRZZRc�� B�Z R�ZRR�ZRc�� B�Z�R Z�R�ZRZZ�RZRR�ZRZ!!)�  � 1� T1JB!9J  )1  BJBZ������������������������������޵�Zk�����޽�����������������������������ƽ��{scR1!!� ���� �����1�  B���� )��� �� Z����{  )����� ������ƽ����B  9������ ������� B����B  Z���{ �����B� �����c� c��� �� Z�����J 9�����B� !����޽� {����!� ���� �� !������� !����	!!!� )1  !)R1�)ZZ  1B!Jk���{������������������������������0��������������������������������ֽ�{sskZRB) �  � �ck��� )�c���  �c�s� J�cs��� �c{�R� J�cs�  Zֵ�������c {� Rcc{�{� B�csֽ� R�c�B� k�B )�ck�� )cck�B� �cs1�  �c{��  R�c������� J�ck��  1�c��� )�cs��� J�cs��� J�c	!!!� )1  !)R1�)ZZ  1B!Jk���{������������������������������0��������������������������������ֽ�{sskZRB) �  � �Zc���  !�Z��� �Z�s� B�Zk���  �Zs�R� B�Zk�  R֭�������Z {� JZZ{�{� 1�Zkֽ� J�Z�B� c�B  !�Zc�� !ZZc�B�  �Zk1�  �Z{��� J�Z������� B�Zc��  !�Z���  �Zk��� B�Zk��� B�Z	!!!� )1  !)R1�)ZZ  1B!Jk���{������������������������������0��������������������������������ֽ�{sskZRB) �  � ������ ����ޭ�  s���s� )�������� Z�����R  ����ֽ  ������������ ��  �����{� {�������� ����B  9���B ������� J�����B� c����1  9������ ���� ������ �������� )������ �������� �������� �����	)!!!� 	!!�!9))R{Z BJ9s��������������������������������������������ƽ�������ֵ��skcRJJ1� �c����1�  )�cs�� ��ck��!� J�c���c� ccs���  9�c �� 	B{RBBk�c� 
J޽��  !�ck���B� 9�cs{� �� )�c��� Zc��� �c �� 
����� 1�cޜBB1� 9�c���1�  J�c����  !�c���R� B�c���R� B�c	)!!!� 	!!�!9))R{Z BJ9s��������������������������������������������ƽ�������ֵ��skcRJJ1� �Z{���1�  �Zk�� ��Zc��!�  B�Z{��c� ZZk���  )�Z �� 	B{RBBk�Z� Bֵ��� �Zc���B�  1�Zk{� �� �Z��� RZ���  �Z �� 	����� !�Z֜BB1�  1�Z���1�  B�Z����  �Z���R�  9�Z���R�  9�Z	)!!!� 	!!�!9))R{Z BJ9s��������������������������������������������ƽ�������ֵ��skcRJJ1� ������1� )������� c������!� {�����c� )������� !���� 	1B{RBB����  �����  Z��������B� c����{  ��� ������� ������ s��� �� 
������ Z������BB1� �������1� {������ 9������R� c������R� c����91!��- 1991!RssZ 9Z��������ν�������� ��� ���0�������������������������������޵��{skskZkZ1! � �c��J� Jcck��  1�c��c� !Z�c��� Jc���  B�c� Z�c B� 	JBZ  Z�cs�1�  �c �� J 1�c��� 1c�B�  �c Z� 	){{c Z�c�1�  �c�s� Z�cν� J�c��� Z�c��� Z�c91!��- 1991!RssZ 9Z��������ν�������� ��� ���0�������������������������������޵��{skskZkZ1! � �Z��J� BZZc��  )�Z{�c� R�Z��� BZ{��  9�Z c� R�Z B� JBZ� R�Zs�1�  �Z �� B )�Z��� )Z�B�  �Z R� 	!{{c  R�Z{1�  �Z{s� R�Zν�  B�Z��� R�Z��� R�Z91!��- 1991!RssZ 9Z��������ν�������� ��� ���0�������������������������������޵��{skskZkZ1! � ����Z�  !����� {����c�  ������ 9����� ��� s�  J���  !kBk  B������J� )���� )c 9������� B���J� ���{  c{{c !�����9� R��֔�  k���� 9���ޥ�  {��ޥ�  {���1)!!��*    9BJ9!JZkJ Bs�����������ƽ���������"�������޽����������������������޵���c
ZscckJB! !� �c � JccZ9�� )Z�c� )�c Z�  RcR1�  B�ck�1� R�cR� �R�cR1!� R�c R� R�cs� ZcJ� 9�c9� J�cR�  !RccR) ��c )� R�cJ� !�cJ� !�c1)!!��*    9BJ9!JZkJ Bs�����������ƽ���������"�������޽����������������������޵���c
ZscckJB! !� �Z � BZZR1� !R�Z� !�Z R� BZJ)� 9�Zc�1� J�ZJ�  J�ZR)� B�Z R� J�Zs� RZB�  )�Z 1� B�ZJ� JZZJ!� �Z )� J�ZB� �ZB� �Z1)!!��*    9BJ9!JZkJ Bs�����������ƽ���������"�������޽����������������������޵���c
ZscckJB! !� ��! 9Z)) !Z�����s�JZ99BZ��� �� !���� �� ){J9��Υc9ZZB ������B�  {����9� ))Z�����cc� J��� s� ������ 9��Μ9)����{9� !������J !9sB9������Z!)ZJJ�Z���� B� s���	�BZ))!)����	�BZ))!)�����1)!)��  )B9119RcB !R{������	��ν������������+�ν����������������������޵���{�kBZJZZ!!)� �c
J91)))R�c
R91199!9)Z�cZ!� B�cZ9J)J�cB19919�c���� !�cR9))R�cB9)�Z�c1 �1�cZ)�R�c9!!1�cZ9!�B�c	B))9J)9�cB)!191�9 J�c B� !�c191)))9�c191)))9�c1)!)��  )B9119RcB !R{������	��ν������������+�ν����������������������޵���{�kBZJZZ!!)� �Z
B))!J�Z
J1))1))R�ZR� 9�ZR 1BB�Z9))1))�Z���  �ZJ1 !J�Z91!� R�Z 1� )�ZR!   J�Z1)�ZR)� 9�Z9�)91�Z9!)1)�1 B�Z9 � �Z�)1�Z�)1�Z1)!)��  )B9119RcB !R{������	��ν������������+�ν����������������������޵���{�kBZJZZ!!)� ��Ƶ���������ƽ��ƽ�ƽ�������������������ƽ���ƽ�ƭ�����
��9��{ZZJ!� c����c�������Ɯ����s)Bss������{���s����ƽ�����ƽ�BJ{�����ƽ����ƽ��� ���ƽ��������������������������������X!!))  �(  91!))JcB 1k����֭��{�������������������������������έ�����������������������޽���B19JZB!J!� �cRRccZ�cRR�c R�c��1 BJcRB99)!  �cZJJ�c Z�cR1)BBZ�cRJcZRJ�cJ)BZ�cZRZccR�c!!))  �(  91!))JcB 1k����֭��{�������������������������������έ�����������������������޽���B19JZB!J!� �ZJJZZR�ZJJ�Z J�Z��1 9BZJ911!�  �ZRB9�Z R�ZJ)99J�ZJ9ZJJB�ZB!9R�ZRJRZZJ�Z!!))  �(  91!))JcB 1k����֭��{�������������������������������έ�����������������������޽���B19JZB!J!� ����1 Ƶ�������  �����Ʒ������� ��� ���ƽƁ�����!�   � %!))!9BR) B{���Υ�kkcZk������������������ν���{k��������ޥ�������������������������ޭ��J)9BJ�B !�� �c k�c k�c k�ckkck�ckcckckk�ckk�ckkcckckckckcckcck�c k�ckk�ckcckcck�ckk�ckcck�ckc��  9k�ckc��)cckkcckkcckkcck�ckckcck�c	kckkcckkck�ckckkckckckcck�c	kcckkcckck�ckk�ckk�c k�ckckk�c k�c k�ckk�ckcck�c k�c�kckcck�c k�ckckkcckcckkcckck�ckkck�c k�ckcckc�kcck�c k�c k�c k�ckkck�ckcckckk�ckk�ckkcckckckckcckcck�c k�ckk�ckckk�ckcckck�c k�ckcck�ckcc�!�   � %!))!9BR) B{���Υ�kkcZk������������������ν���{k��������ޥ�������������������������ޭ��J)9BJ�B !�� �ZcZ�c Z�cZZ�c�ZcZc�Z�cZZ�cZccZc�Z�cZcZc�ZcZ�cZccZ�c�Z�cZccZcc�ZccZZcc�ZcZ�c Z�cZcZccZZc{�  1Z�cZ��!ccZcZcc�ZcZcZc�ZccZZcZZccZZccZc�Z�c	ZZcZcZZcZZ�cZc�Z�c Z�cZZ�c Z�cZc�ZcZZc�ZccZcZZccZcZZccZcZZccZccZZ�c Z�c Z�cZccZZ�c�Z�cZZcZcZc�Z�c Z�cZccZZc�Z�c Z�cZZcZZcZ�cZc�Z c�ZcZcZccZZcZZcZ�c Z�cZZ�c�ZcZc�Z�cZZ�cZccZc�Z�cZcZc�ZcZ�cZccZ�c�Z�cZccZcc�Z�cZcZcZ�c Z�cZZccZ�c Z�c Z�c Z�!�   � %!))!9BR) B{���Υ�kkcZk������������������ν���{k��������ޥ�������������������������ޭ��J)9BJ�B !�� �� ��� ��� ־� ���޽  ���������J���� ֭� ��� ��� ��� ��� ��� ��� ��� ְ� ���))�� 	!!�9! Z{�罌�sJZ�c	s����{{���������޽���kcssc������ޥ�����������������������Υ��B)99Z{cJ!� �k�  1{{kk����JZ�k�k�k))�� 	!!�9! Z{�罌�sJZ�c	s����{{���������޽���kcssc������ޥ�����������������������Υ��B)99Z{cJ!� �c �� )sscc����9R�c�c�c))�� 	!!�9! Z{�罌�sJZ�c	s����{{���������޽���kcssc������ޥ�����������������������Υ��B)99Z{cJ!� ���������������������������������� ����������� �������������������������������������  J���������Ƶ��������������������������� ��� ��������������� ������������������������������� �����������������
������������������ ��� ��� ����������� ������������������������������������������������������ ����������� ����������������������������� ����������� �	�!��  � 1!�119 s��΄s�J!BZcc����ޥsZ{�����������ν�������{J{����ν����������������������ν���J9BBc�{R!� �k{  ������Z !!J�k�k�k�!��  � 1!�119 s��΄s�J!BZcc����ޥsZ{�����������ν�������{J{����ν����������������������ν���J9BBc�{R!� �cs  ������Z B�c�c�c�!��  � 1!�119 s��΄s�J!BZcc����ޥsZ{�����������ν�������{J{����ν����������������������ν���J9BBc�{R!� ���������� ������������� ���������������������������������� ���������������� �������s B����� ��������������� ��� ��� ��� ������������������ ����������������� ���������� ������� ����������� ��������� ����������������� ���������������� ������������� ���������������������������������������������������� �������� !� �� 'B)19J s{��ckJ!1RRcs�έ�Δ9B��������-�������Δs������{R{������ֽ������������������������޽�Z��ZZB9Js�R� �kZ!1� !Z�k�k�k !� �� 'B)19J s{��ckJ!1RRcs�έ�Δ9B��������-�������Δs������{R{������ֽ������������������������޽�Z��ZZB9Js�R� �cR!  !� R�c�c�c !� �� 'B)19J s{��ckJ!1RRcs�έ�Δ9B��������-�������Δs������{R{������ֽ������������������������޽�Z��ZZB9Js�R� ���csc�� R��΁ց֢��!�� 9!�199 s���R)9Zc�R
{�{���Z)R����%�����ֽ��Ƶ�R9Rs���ֽ��c������ֽ��������	������������������ZB{{cBBJ9BRcZ� �kkJBJZ1�!R�k�k����k�k!�� 9!�199 s���R)9Zc�R
{�{���Z)R����%�����ֽ��Ƶ�R9Rs���ֽ��c������ֽ��������	������������������ZB{{cBBJ9BRcZ� �ccB1BR)�B�c�c����c�c!�� 9!�199 s���R)9Zc�R
{�{���Z)R����%�����ֽ��Ƶ�R9Rs���ֽ��c������ֽ��������	������������������ZB{{cBBJ9BRcZ� �� ��������΁����������� �� (1 !)B) cs{B!{���sB)k{ks��{!)����#����Ɯ����kR9J��Z9Js����������ε���������ƽ����������������sJ9ccJcR!!c{B� �k�k������s�k֭ 1�k�k� �� (1 !)B) cs{B!{���sB)k{ks��{!)����#����Ɯ����kR9J��Z9Js����������ε���������ƽ����������������sJ9ccJcR!!c{B� �c�c������k�cέ 1�c�c� �� (1 !)B) cs{B!{���sB)k{ks��{!)����#����Ɯ����kR9J��Z9Js����������ε���������ƽ����������������sJ9ccJcR!!c{B� ���� ��������� k�����!)!�� (1)9R! )Rk{9 B���!  RJ�csƌ!����O����֥����kZs���RJ)Z���������ε������������������������ޭ������{B)RcRRZ1)9)� �k�k���RB1 !c�k�R  !c�k�k!)!�� (1)9R! )Rk{9 B���!  RJ�csƌ!����O����֥����kZs���RJ)Z���������ε������������������������ޭ������{B)RcRRZ1)9)� �c�c���RB1 !�c�R  Z�c�c!)!�� (1)9R! )Rk{9 B���!  RJ�csƌ!����O����֥����kZs���RJ)Z���������ε������������������������ޭ������{B)RcRRZ1)9)� �������RB1 )Ɛ��R �΁���l!� 9�71sZ9Z�{1 !R��)  B!{Ɣ��{)��������筥����ss���c� )JJ��������������������������������޽�k����c91Jkckk!!)� �k�k{{� J�k�B  B�k�k!� 9�71sZ9Z�{1 !R��)  B!{Ɣ��{)��������筥����ss���c� )JJ��������������������������������޽�k����c91Jkckk!!)� kc�kckkcckc�kck�ckkcckc�kcckkcck�c k�c k�c	kcckckcckc�k�c!kkcckkckckkcckcckcckcckckckckkckkc�kcckkc�k�ckkckk�ckck�ckkcckc�kck�ckkcc�kck�ckcckckkcckkck�cs{�  B�c�kckk�ckkcck�c�k c�k�ckkckck�ckck�ckckkckcc�k�ckkckckkc�k c�k�ckc�kcckk�c�k�ckk�B  9ckkc�kck�ckkc�k�c�k	cckckkckkc�k�c�kckkcckc�kck�ckkcckc�kcckkcck�c k�c k�c	kcckckcckc�k�ckkcckkckckkcckcckcckcckckckckkc�k�ckkcc�k�c�k�ckckc�k!� 9�71sZ9Z�{1 !R��)  B!{Ɣ��{)��������筥����ss���c� )JJ��������������������������������޽�k����c91Jkckk!!)� ���� �� BƑ��B  �Ɓ������ 1�X9sJ1cksB9cB��  )��s��{B������������ƽ�����) 1 s�J�����������������޽�������������ֽε�{����c99ZkRRJ� �s�sk)� c�s�{  B�s k�s�s�� 1�X9sJ1cksB9cB��  )��s��{B������������ƽ�����) 1 s�J�����������������޽�������������ֽε�{����c99ZkRRJ� �k�kc)� Z�k�{  9�k�k�� 1�X9sJ1cksB9cB��  )��s��{B������������ƽ�����) 1 s�J�����������������޽�������������ֽε�{����c99ZkRRJ� �����R� )�Β��{ �Ɓ���� �� !)�9k91cZs�Zc9k�J� 91R��Bs�c!���������ν��絜�����B   ��������������������������������֭����sks�ssR9B�JB  � �s�s {� Z�s�� J�s�s �� !)�9k91cZs�Zc9k�J� 91R��Bs�c!���������ν��絜�����B   ��������������������������������֭����sks�ssR9B�JB  � �k�k s� R�k��  B�k�k �� !)�9k91cZs�Zc9k�J� 91R��Bs�c!���������ν��絜�����B   ��������������������������������֭����sks�ssR9B�JB  � ������ )�Β�ޜ J�Ɓ���e�� -)1)Z!!)Js�Z!1JJssB1R�ƔZ���1R�������޽���֭������R� )�����������������������������������ƽ�����{ss{kccJ1RB11� � �s�s{�����s�{� Z�s����s����{�s���s�����s���s{� k�s���s�s�� -)1)Z!!)Js�Z!1JJssB1R�ƔZ���1R�������޽���֭������R� )�����������������������������������ƽ�����{ss{kccJ1RB11� � �k�ks�����k�{� R�k����k����s�k���k{��{�k�{�ks� c�k�{�k�k�� -)1)Z!!)Js�Z!1JJssB1R�ƔZ���1R�������޽���֭������R� )�����������������������������������ƽ�����{ss{kccJ1RB11� � �����������{� )��������������������������� Z����ށ���z �� N !!  !B9BJ��!19Bk{{��Δ���{k���������Ƶ���Ƶ�����ZJ9JR�������������������$ν����������Ƶ������{��{cc��Rkc1)1)11� � �s�s����Ƶ�s�������{ss���{{css��� ����޽{�s�����ƭ�s���ν�ss��������s��������ss��޽���s��������{�s�����{�s����ƽ�s� s����﵌�����{�s����޽��s�s �� N !!  !B9BJ��!19Bk{{��Δ���{k���������Ƶ���Ƶ�����ZJ9JR�������������������$ν����������Ƶ������{��{cc��Rkc1)1)11� � �k�k{��޽��k{������skk���{{ckk��� ����޵s�k����޽��k���ν�kk��������k��������kk������k��������s�k�����s�k���޽��k� k����בֿ�����s�k����޵��k�k �� N !!  !B9BJ��!19Bk{{��Δ���{k���������Ƶ���Ƶ�����ZJ9JR�������������������$ν����������Ƶ������{��{cc��Rkc1)1)11� � ���������������������
���{{���޽� Z��������������������ν������������������������޽�������������������� �������� ����������������������ށ���� �� �  1  � )91R�R1)!BJJk�s���)s���"�����޽������{Rs����sks����������������罽���������ƭ�����{s��sZJ��ZZR!)9)� � �s�s�ޜBBsZss�ޭBRcB)ks֜� 1k��� 	�{  BZk�s	���kBs���s�s� Zs�!�  9�s�B� Js��� ss{���R  1BZ��s����c kss��RBBsR�s� 1!s�BB !��{ kkss{���Z 1B{���s�s �� �  1  � )91R�R1)!BJJk�s���)s���"�����޽������{Rs����sks����������������罽���������ƭ�����{s��sZJ��ZZR!)9)� � �k�k�ޜBBsZkk�ޭBRcB)ck֜� )c��� �{  BRc�k	���kBs���k�s� Rk�!�  9�k�B� Bk��� kks���R  1BZ��k����c ckk��RBBsR�k� )k�BB !��{ kckks���Z 1B{���k�k �� �  1  � )91R�R1)!BJJk�s���)s���"�����޽������{Rs����sks����������������罽���������ƭ�����{s��sZJ��ZZR!)9)� � �����ޜBBs����ޭBRcBB����� ��޽� 	J��{  Bs���	���kBs����ބ� {��)�  k����B� �����  1��
���R  9BZ�������c �����RBBss��� ����BB !��{ ����	���Z 1B{�ށ���W�� � 9!�   �!)1J1JZJ1)19BZ������B!{������� �����Μ�����{ck{sk���{��Ƶ���������*���������������絥��������{���J9k�ZZB91�  � �s�s �� )c{9� !R�R� )��� � !Z�s����  � R�sZB� k!!� k�s�!� !�� Z��1� �  R�s� Js�B� !c�s� 99ss1� cs��c  � !Z�s�s�� � 9!�   �!)1J1JZJ1)19BZ������B!{������� �����Μ�����{ck{sk���{��Ƶ���������*���������������絥��������{���J9k�ZZB91�  � �k�k �� !Zs9� J�R� !���  � R�k����  � J�kZB� c!� c�k�!� �� J��1�  J�k s� Bk�B� Z�k� 1)kk1� Zk��c  � !R�k�k�� � 9!�   �!)1J1JZJ1)19BZ������B!{������� �����Μ�����{ck{sk���{��Ƶ���������*���������������絥��������{���J9k�ZZB91�  � ���� �� {��J� ���R� s�޽� c� k������  )  ����R� 	B�BJcB  ����!� J��� ����1� )J  ��� �� c���B� {���� ����9� 	����c  )� 1�����y�  � )!  �  )B)1�{ZZJcs��ε���B){�������������ƭ�����ε��������6ν���������������ֽ��������ֽ��������ƥ���{R1JkJR19!�  � �s�sR� !skR� 1s�� !��� � ss��s� !�  1�s R� cR)kB�  1�s� k{� 9�{� B)� 9ss�J� !ssR�  !�scB1ss�)� 
s�1  1�  B�s�s�  � )!  �  )B)1�{ZZJcs��ε���B){�������������ƭ�����ε��������6ν���������������ֽ��������ֽ��������ƥ���{R1JkJR19!�  � sskk�sks�ksskks�kss�kss�k	sksskksskk�skss�ksk�s k�s k�sks�k�skkss�kskkss�kskksk�s�kskkskskss�kssks�k�sksskkskkskkssks�kssJ� kkR� )s�� ��� � ks��s� �  )�k R� ZJ!cB� !sks�k s�k s� c{� 1�{� 1!� 1ks�J� kkR�  �skc9)sk�)� k�1� )� 
9sksksskssk�sksk�skk�sks�ksskks�kss�kss�k	sksskksskk�skss�ksk�s k�s k�sks�k�skkss�kskkss�kskkskskks�k�s�k s�kss�k s�k�  � )!  �  )B)1�{ZZJcs��ε���B){�������������ƭ�����ε��������6ν���������������ֽ��������ֽ��������ƥ���{R1JkJR19!�  � ������ 9���k� 9��ֽ� Z��޽� !s�9� �����s  !���J�  ��� {� 	��ν�c  ��� �� ��֌� ���{� J���c  ����J� )��ք� 9����ƽ���9� ���1  B���!  Ɓ�����  � �   Jc!1��{k����%����s99��������ν�������֭������������νƽ������������&ε�������Ƶ�����kZ��������s9BZkZJ!)!�  � ss{{s�{s{{s{{�s�{s{{s�{ s�{�s�{s{s{s{{s{�s{{s{s{s{{s{s{{s{s�{�s {�s{{s{{ss{s�{	s{{ss{s{ss�{ss�{ss{s{�s {�s�{�s�{ �� cs�� c��� 1c��� B�� Bs޽� Rs�� 9{ss�� Bs{�{� Bs{{s�{�s{s�� Bs�B  ��� !cs�)� !{{�� Z{{�� c{s{�s{ss�� � �{� cs�� Rs{s�{s{�s�{s{{s�{s{{s{{�s�{s{{s�{ s�{�s�{s{s{s{{s{�s{{s{s{s{{s{s{{s{s�{�s {�s{{s�{	s{{s{s{ss{�s�{s{ss{s�{
ss{ss{s{�  � �   Jc!1��{k����%����s99��������ν�������֭������������νƽ������������&ε�������Ƶ�����kZ��������s9BZkZJ!)!�  � �s�s �� Zs�� Z��� )Z��� 1�� 1s޽� Js��  1�s �� 9ss�{�  9�s �� 9s�B� ��� Zs�)� ss�� Rss�� Z�skk�s ��  � �{� Zs��  J�s�s�  � �   Jc!1��{k����%����s99��������ν�������֭������������νƽ������������&ε�������Ƶ�����kZ��������s9BZkZJ!)!�  � ������ )����� Z����  )����޽� ����� ������ ������ )��� �� �����{� Z��� �� )����B  B����� B�����)  {���� J������ )���� �� s�Z� B��{  ���֭� Z΁�����  � � %   ZZJ)9s{s{�����{����1R����ֽ����M��������Ɣ����������Υ�����������������ƽ������ƭ���Υsc99s�������sBB�sB)!1� �{�{�k� c{�B� {ν  Z{{��� J{�1� )��B� ){{�B� Z{{�� B{{��� �{�)� Z{�{  !��� k{{��� )s�{� s{{��� c�{ �� )kJ  �  B{��� s�{�{�  � � %   ZZJ)9s{s{�����{����1R����ֽ����M��������Ɣ����������Υ�����������������ƽ������ƭ���Υsc99s�������sBB�sB)!1� �s�s�k� Zs�B� sν� Zss��� Bs�1� !��B� !ss�B� Zss�� 9ss���  �s�)� Zs�{  ��� css��� !k�{� kss��� Z�s �� !cB� �� 9s{�� k�s�s�  � � %   ZZJ)9s{s{�����{����1R����ֽ����M��������Ɣ����������Υ�����������������ƽ������ƭ���Υsc99s�������sBB�sB)!1� �����k� ����B� J���  �����޽� !�����1� c���B  )�����B� c����� !����ޜ� Z����)� )����{ )����  !������ ���{� J���ޜ� )���� �� 9��֜!���  Z������  ������ ��  �  �  �  !J9cBBRs�������c��քs��ƽέ���������έ�������޽��������������������޽έ����ƥ�c1)k��{s��kJ{�R9)!�  � �{�{s� J{�B� s�c )�{��� J{{�{� ν� ����B� {��� Z{{��� �{ �� 
c{�9c�c� R�{ �� R��� s{{��� c�{ �� 	c{{sBc{� ����� 1�{�{ ��  �  �  �  !J9cBBRs�������c��քs��ƽέ���������έ�������޽��������������������޽έ����ƥ�c1)k��{s��kJ{�R9)!�  � �s�sk� Bs�B� k�c  !�s��� Bss�{� ν� {���B� s��� Rss���  �s �� 
Zs�1 Z�c� J�s �� J��� kss��� Z�s �� Zssk9Z{� �����  )�s�s ��  �  �  �  !J9cBBRs�������c��քs��ƽέ���������έ�������޽��������������������޽έ����ƥ�c1)k��{s��kJ{�R9)!�  � ����	�  )����B� ���c !������ ������{� s��� )�����B� 9����� )������ c��� �� ����J����c� Z���� �� ��ތ� )����� )���� �� Z����Ƶ�{� Z������ c�����p ��  � ! � )))Z�kR���ƽ�ƽ�����RR����ƽ����������������������ƽ�������	������Ƶ����ƽ�������޽kR1Bk��k{��kk�{J91�  � �{�{ �� c��!� k�) Z�{��� 9�{��� !�c� ��{B� k��� c{{��� �{�{� 
J��9J{�B� �{ �� 1{�� c{{��� c�{ �� �{��!� ��{!� �{�{ ��  � ! � )))Z�kR���ƽ�ƽ�����RR����ƽ����������������������ƽ�������	������Ƶ����ƽ�������޽kR1Bk��k{��kk�{J91�  � �s�s �� Z��!� c�)  R�s��� 1�s��� �c� ��{B� c��� Zss���  �s�{� 	B��19s�B�  �s �� )s�� Zss��� Z�s ��  �s��!� ��{!�  �s�s ��  � ! � )))Z�kR���ƽ�ƽ�����RR����ƽ����������������������ƽ�������	������Ƶ����ƽ�������޽kR1Bk��k{��kk�{J91�  � ���� �� ����!� ���1 c������ ����޽� ���c� s��{B� J����� c����޽� �����{� 
{���1s����B� s��� �� {�ֽ� )������ )���� �� J�����!� !{��{!� {�����7��  � �  � 	19�����������ƽ��9 1������Ƶ����������������������������������Υ����ƭ����$ν����{s���֜J�Z )J�sk��{c��kkJB1  � �{�{�R  k�J� R�  �{��� J�{��� )�� s��� �{ �� )�{ �� 
)� c!s{�B� !�{��� )��� c{{��� c�{��� s�{��� )�{�{��  � �  � 	19�����������ƽ��9 1������Ƶ����������������������������������Υ����ƭ����$ν����{s���֜J�Z )J�sk��{c��kkJB1  � �s�s�R� c�J� J�  �s��� 9�s��� !�� k��� �s ��  !�s �� 	!� Zks�B�  �s��� ��� Zss{�� Z�s{�� k�s��� �s�s��  � �  � 	19�����������ƽ��9 1������Ƶ����������������������������������Υ����ƭ����$ν����{s���֜J�Z )J�sk��{c��kkJB1  � �����R  ���Z� Bέ ������ ������� ���� s���� ���� �� ����� 
Z��� ν���B� ������� Z���� B����޽� B������� )���޽� ������V��  � �  � 	){���ƽ���������֌!Z�������έ������������������������������������ν����"ν����cc���Ɯk��!)J�{{��sk�{c{)JJ� �{�{��� Zs !� 9 J�{��� B�{��� )�1� �!{ƭ� 9�{��� )�{�R� 	�! !J{{�c� )�{��� {�� s{{��� s�{��� c�{�� ��9�{�{��  � �  � 	){���ƽ���������֌!Z�������έ������������������������������������ν����"ν����cc���Ɯk��!)J�{{��sk�{c{)JJ� �s�s��� Rs !� 9 9�s��� 9�s��� !�1� sƭ� 1�s���  !�s�R� 	�! 9ss�c�  !�s��� s�� kss��� k�s��� Z�s�� 1�s�s��  � �  � 	){���ƽ���������֌!Z�������έ������������������������������������ν����"ν����cc���Ɯk��!)J�{{��sk�{c{)JJ� �����  9�{ J{  {9 s������ ������� c��1�  )�Z{��sc����  ������� �����R  !��! �����c� ������ B�ֽ� Z������ s������ )������  B�Z��c{�Ɓ���3��  �  � !���ƽ���������Z1��ƽ������������������������������.ƽ�ν����������έ�k������Z�sR)RR��������RcR1J!� �{�{��� R� )s�{�{� B�{��� ��� �JRc��Jk{�s� R�{��� 1�{��� 1  J{{�c� )�{��� ��� �{��� !�{��� c�{�s� !�JRk�sJs�{�{��  �  � !���ƽ���������Z1��ƽ������������������������������.ƽ�ν����������έ�k������Z�sR)RR��������RcR1J!� �s	{{ss{{s{s{�s�{�s {�s�{�s�{�s{s{{ss�{ss{ss{{s{{s{�s{ss{ss�{s{{�s{{�s�{ss{s{s{{�s�{�s{s{{s{s{s{{s{ss{ss{�s{ss{�s��� J� ss{ss�{� 9s{s��� ��� �BZ��Bc{�s� Jss{��� !s{s{�s�{s{{��� 1  Bs{�c� !s{s{��� ���  �{��� {�s{��� Zss{{s�s� �BJc�kBs{�s {�s {�s	{{ss{{s{s{�s�{�s {�s�{�s�{�s{s{{ss�{ss{ss{{s{{s{�s{ss{ss�{s{{�s{{ss{s�{�s {�s{ss�{ss{{s{�s{s{s��  �  � !���ƽ���������Z1��ƽ������������������������������.ƽ�ν����������έ�k������Z�sR)RR��������RcR1J!� ����޽� ) ��c� �����{� ������ Z��� ������������s  )������� ����ތ� 	!B  �����c� ������ c���� Z����� ����ޭ� c�����s� 9��������Ɓ����� � � Bs���ε���������s1)k�����ֵ����������� ���5����ν���������{sk�ƔZs���ֵsJ�ZJJ!ZcZ�������kkZ9R1 � �{ ��{ ��{ ��{ ��{ ��{���{ ��{ ��{�{�{{��{ ��{�{{�{{�{{���{ ��{�{{��{ ��{���{ ��{ ��{�{{�� Z�� J�{�{� B�{��� ��� R�{�{���J�{�s� c�{ �� 1{��{ �� J{{��� 9�{��� ��� �{�{� 1�{��� �{�{�{� k�{��kZ�{ ��{ ��{ ��{ ��{ ��{ ��{ ��{���{ ��{ ��{�{�{{��{ ��{
�{{�{{�{{���{���{ ��{ ��{� � � Bs���ε���������s1)k�����ֵ����������� ���5����ν���������{sk�ƔZs���ֵsJ�ZJJ!ZcZ�������kkZ9R1 � �{�{ �� J�� B�{�{�  9�{��� ��� B�{���B{{�s� Z�{ ��  )�{ �� B{{��� 1�{��� ���  �{�{�  !�{���  �{�{� c�{��kZ�{�{� � � Bs���ε���������s1)k�����ֵ����������� ���5����ν���������{sk�ƔZs���ֵsJ�ZJJ!ZcZ�������kkZ9R1 � ���� �� )���� B�����{� ������ Z�޽� )�������{���s  ���� �� ���� �� ����ޭ� ������ ����� s����{� ������� �����{� Z�����{�����) � � � +J����ֵ��k�����sJc���������ƭ�{������������5�����ƽ�������{ZJZ��kks����֜��kk�k)�sZ�Υ���{RkccJ � ����1� c�B� s���{� 1��Ɯ� ��� c�����  Bs�{� R����� 1���9� Z���  J��ޭ� 9��� !���{� 1���� 1�� �� ����J !B��� � � � +J����ֵ��k�����sJc���������ƭ�{������������5�����ƽ�������{ZJZ��kks����֜��kk�k)�sZ�Υ���{RkccJ � �{�{�1� Z�B� k�{�{�  !�{��� {�� Z�{���  9k�{� J�{���  )�{�9� J�{��  B�{ޭ� 1���  �{�{�  !�{��  !�{ ��  �{��J 9�{�{ � � � +J����ֵ��k�����sJc���������ƭ�{������������5�����ƽ�������{ZJZ��kks����֜��kk�k)�sZ�Υ���{RkccJ � �����1� {���B� �����{� c����� 9���� )������� ���{� ������� �����9� s�����  c������ 9��޽� �����{� s������ ���� �� Z�����J BƁ����� �  � 9�����������Ɯ�1)����������ν{s�������������ƭ��0�����cJ9{�{�޵����֜��B���)��Z�ֽ���{Rs{�s!   ���� Z� )��B� 1���B�  ���{� k�k� c����� k�s� J����� 1��� !�� s� !{����B� k���  1���{� !����!� 1���1�  �����  k���� �  � 9�����������Ɯ�1)����������ν{s�������������ƭ��0�����cJ9{�{�޵����֜��B���)��Z�ֽ���{Rs{�s!   �{�{ Z� {�B� !�{�B�  �{�{� c{k� Z{{��� c�s� B�{���  !�{�� �{ s� s{{��B� c{��  !�{�{�  �{��!�  !�{�1�  �{���� c�{�{� �  � 9�����������Ɯ�1)����������ν{s�������������ƭ��6�����cJ9{�{�޵����֜��B���)��Z�ֽ���{Rs{�s!   ���������������������������� ��� ��� ���������������� ���������� ��� ��� ��� k� ����B� )�������B� !����{� 9���k  ������� s���s� ������� J��������� ��� � )�����ބ  ������B  ������ 9����{� B��������!� �����1  9����
�� ���������������������������������������� ��� ��� ���������������������������� ���#� �  � B������ֽ����ƵƥR����������Ƅs������8���Ɯ��������ZBJ����������ޭ��!���ksƌ��ֽ��sBc���9  ����	�R  Z���� c����1� c���B� k��  s����) 9��� J����R� s� �� c�� J� ������ B���� ����{� s����� 1�� �� 
����� 9���� �  � B������ֽ����ƵƥR����������Ƅs������8���Ɯ��������ZBJ����������ޭ��!���ksƌ��ֽ��sBc���9  �{�{�R� R{{�� Z�{��1� Z{��B� c{�� k����) 1��� B{{��R� k�{ �� Z�{ J� {����� 9{{�� �{{�{�  k�{���  )�{ �� 	����� 1�{�{� �  � B������ֽ����ƵƥR����������Ƅs������8���Ɯ��������ZBJ����������ޭ��!���ksƌ��ֽ��sBc���9  ������	�������������� ����� ����������������������� ���������������������������������������� ��� ����������������������	�Z  ����޽� Z�������1� {����B� B����  Z�����1 1����� ������R� s�����������������ޭ  !������Z  9������ ������� Z����{� ���������� c��������� ������ Z���������������������	�������������� ����� ����������������������� ��������������������������� ��������������������� �����]� �  � R������ƥ��ƽ���{k�����������Ɣ{��{����8���Ɯ��������cBR��������������Z���B�������΄RR���Z  �����{  1���!  ����� )��s� s��� 
{{s  �R� !���Z� {���1 J��1  RJR� 1��s� k����  ����� s�� c� 	){{c {���� �  � R������ƥ��ƽ���{k�����������Ɣ{��{����8���Ɯ��������cBR��������������Z���B�������΄RR���Z  �{�{�{  !�{�!  �{��� {�s� k{{�� 
{{s  �R� {{�Z� s�{�1 B�{)  RJR� !�{s� k{{��  �{��� k�{ c� 	!{{c s�{�{� �  � R������ƥ��ƽ���{k�����������Ɣ{��{����8���Ɯ��������cBR��������������Z���B�������΄RR���Z  �� ��� ��� ��� ��� ��� ������� ��� ��� ��� ��� ��� ����{  �����!  ������� ��ބ� {��޵� 
R{{s ���R� ����k� k��� ����1 �����R  9kJZ� s������ )�����  Z������� B����  c{{c !����� ��� ��� ��� ��� ��� ������� ��� ��� ��� ��� �����  �� 9���� ���������1{�����
��Υ���������(���Ɣ�������scJk�ƽ�����������Ό��9{�������scs�֌B ��߄c J��J  9���1� Z�s1� )��c� B�)�  9��J)�  !{��k B��Z� ){��s� 1s�J� !s��s!� c��J� c����  �� 9���� ���������1{�����
��Υ���������(���Ɣ�������scJk�ƽ�����������Ό��9{�������scs�֌B �{�{c  B�{B  1�{�1� R{k)�  �{Z� 9{)� ){{B)� s�{�c 9�{R�   s�{k� )s{B� k�{k!� Z�{J� Z�{�{�  �� 9���� ���������1{�����
��Υ���������(���Ɣ�������scJk�ƽ�����������Ό��9{�������scs�֌B ����� J����c 9���� J� ��޽R )s����J� ����9  Z{){���{RRZZ J�����s ������  1){������1� ���c� {����	�11  !!�����9� !��΁���� !�ƽ�ƭ���������c�������έ��ƽ�������8��ν��������scZ��������������������1B������ޥ�ks�֜c! ��ބc!{��Zs��{JR��Z1!)!��{R1�B�� J�1cB9��sJ9JJ1s����֔���Z1J��	{c9)1B{�� J�)��Z� 1��J1))J��{B)�Z���� !�ƽ�ƭ���������c�������έ��ƽ�������=��ν��������scZ��������������������1B������ޥ�ks�֜c! {�{�{�� {���{�{�{{�{{��{���{ ��{��{��{{��{�{�{�{�{��{�{{�{��{��{��{��{�{�� {��{�{�{��{	��{�{�{�{��{��{{��{��{	�{��{{�{{��{�{�{���{�Zs{�{�{Rk�{�{{J � J�{R!����{J!� 1{�{J !Z9){��kB)BB)s�{�{{��{��Ό{{��Z1B{{��{�{���{Z1 !1{{�{�{��{B ��R�!{��{��{B!B{��{��{s9� R{�{{���{��{��{��{�{�{�� {���{�{�{{�{{��{���{ ��{��{��{{��{�{�{�{�{��{�{{�{��{��{��{��{�{�� {��{�{���{��{{��{{��{��{��{{��{{�� !�ƽ�ƭ���������c�������έ��ƽ�������8��ν��������scZ��������������������1B������ޥ�ks�֜c! ����������������Δ{��{�����ν�����	�ν�ZBs��������������� ��������������R����	�Ɣc���������������ޜs������ƽ�������	�ƽ�BJ{��΁���� !���ƽ���������R!B�������ֵ���֭����������Ƅ{{�����skks���������������c9s�����ޥ��s�֥k! ��݄{{��{s��sZ{{kZ�s9Js��{ks�{{����cZ����k�9k�sZk�sk{��{kZ�c9R{���� !���ƽ���������R!B�������ֵ���֭����������Ƅ{{�����skks���������������c9s�����ޥ��s�֥k! ��݄ss��sk��kRsscR�k)Bk��sck�ss����cZ{���k�)c�kRc�kcs��scR�Z)Js���� !���ƽ���������R!B�������ֵ���֭����������Ƅ{{�����skks���������������c9s�����ޥ��s�֥k! ���������������� �������������������cZ���������������������������ց����� !��Ƶ����������)!s����������
����֔���������ֽ�������{{sc�������������������J�������ks��Δ{)! �������������� ��� ������������� ��� ��� ������������������������� ��� ��� ��� ��� ���������������� ��� ��������� ��� ��� ��� �����1� 	{�Z){��������� ������� ��� ��� ��� ������� ��������� ��� ��� ��������������� ��� ������������� ��� ��� ������������ ������������ !��Ƶ����������)!s����������
����֔���������ֽ�������{{sc�������������������J�������ks��Δ{)! ������1� s�Z)s{����� !��Ƶ����������)!s����������
����֔���������ֽ�������{{sc�������������������J�������ks��Δ{)! �ދ���1  ��{B�ցޖ�{� ,!�������������s!9��������ֽ�������ޔ����������ֽ�s������{�sk��������ޭs�������R�������{R��Δ�9) ���� �� ) c!s����� ,!�������������s!9��������ֽ�������ޔ����������ֽ�s������{�sk��������ޭs�������R�������{R��Δ�9) ���� �� ! Zk����� ,!�������������s!9��������ֽ�������ޔ����������ֽ�s������{�sk��������ޭs�������R�������{R��Δ�9) �ފ� �� !J ��ցޖ�t� 	k��{{������	�{ B����Ƶ��cZRs��΄{��������8���Ɣk��������{s{�������ƭ��Z�����Δcs�������{Rsc��{9! ���� �� !1����� 	k��{{������	�{ B����Ƶ��cZRs��΄{��������8���Ɣk��������{s{�������ƭ��Z�����Δcs�������{Rsc��{9! ���� �� )����� 	k��{{������	�{ B����Ƶ��cZRs��΄{��������8���Ɣk��������{s{�������ƭ��Z�����Δcs�������{Rsc��{9! �ފ� �� 1���ޕ�~� -)k�{k��������έB !k����������������{��������������Δs�������*ss�������ޔ���{���ֽ�s��������kJcB��k) ���� 9� k����� -)k�{k��������έB !k����������������{��������������Δs�������*ss�������ޔ���{���ֽ�s��������kJcB��k) ���� 9� c����� -)k�{k��������έB !k����������������{��������������Δs�������*ss�������ޔ���{���ֽ�s��������kJcB��k) �މ� c� ��ցޕ��� �� 1k�sRs������ֵ��c) )c{{��s�����֭����������������ƌk�����{��sk�����祵�������޽�k������νc1J)��J) Ɍ�����ν���Ɍ{� R������������������������� �� 1k�sRs������ֵ��c) )c{{��s�����֭����������������ƌk�����{��sk�����祵�������޽�k������νc1J)��J) Ʉ������ν���Ʉs� B������������������������� �� 1k�sRs������ֵ��c) )c{{��s�����֭����������������ƌk�����{��sk�����祵�������޽�k������νc1J)��J) �� ��������� ��� γ������������������������ �� ))� )kkZJs����#޵��ε�s9!Js�������������������������������﵄k�������{ck���#ƥƭ����������{s������Ƶc1B9��11 ˌ����s  9s����������������������������R1���ｽsZ��ｽsZ��� �� ))� )kkZJs����#޵��ε�s9!Js�������������������������������﵄k�������{ck���$ƥƭ����������{s������Ƶc1B9��11 ��� ������� ���������������������������s  9s����������������������������������������� ��������������� ������������������ ������������� ������������������������������������������������������������������ �������� ����������������������� ����� ����� �����R1{�����	���ｽsR�� ������ｽsR����������������������������������������������������������
�������������������� �� ))� )kkZJs����#޵��ε�s9!Js�������������������������������﵄k�������{ck���#ƥƭ����������{s������Ƶc1B9��11 ������s  9s�ց������� ������� ����������R9����ｽs����ｽs���p� !��!� 1{ZBB��������������������֭����������������ֽ����������{k������{Z����$����s������֔��k�����޽��J))c�c1  ̌���R�  �������R�����cBB s�����cBB s��R� c���s� 1����s� 1���� !��!� 1{ZBB��������������������֭����������������ֽ����������{k������{Z����$����s������֔��k�����޽��J))c�c1  ̌���R�  �������R�����cBB k�����cBB k��R�  Z���k� )����k� )���� !��!� 1{ZBB��������������������֭����������������ֽ����������{k������{Z����$����s������֔��k�����޽��J))c�c1  �����R� )) )�������{�����cBB ������cBB ����R� B��� ��  ��� ��  ����� )� 9�c99{���޽�������������	�ƥ���������	�������ƽ����������組{�������sR����'ﭽ�R������޽���������Υ�9{�R9  �����
������������� ����������������������  ��!  )���������������������	������������ ���������������������
�����������������������������  R{�������������� B����������������������������������������������������������� B������������� ����B� ��� �����Z� R��������������Z� R���������������������������������������������������� ���������������������� )� 9�c99{���޽�������������	�ƥ���������	�������ƽ����������組{�������sR����$ﭽ�R������޽���������Υ�9{�R9  ͌	���   �  )�����  Js� ��  9�� ��  9��B� {���Z� J��Z� J��� )� 9�c99{���޽�������������	�ƥ���������	�������ƽ����������組{�������sR����$ﭽ�R������޽���������Υ�9{�R9  �����  R�����c R΂���  ���� �� Ʒ� �� ����B� B�����s� �����s� �Ε�d�  � B�{)1{���έ���ν���������筵�����������������ε���� ��� ���8��筌��ƭ����sZ�����甄Rc�ck���֥���������Μ�9B{kB  ͔��� !k{���  k���{  J� J� c�� J� c� k� �� {� R� {� R���  � B�{)1{���έ���ν���������筵�����������������ε���� ��� ���8��筌��ƭ����sZ�����甄Rc�ck���֥���������Μ�9B{kB  Ό���� cs��  c���{�  B�� B� Z�� B� Z��k� {� {� J� {� J���  � B�{)1{���έ���ν���������筵�����������������ε���� ��� ���8��筌��ƭ����sZ�����甄Rc�ck���֥���������Μ�9B{kB  ����� !�������  1փ��{  ����k� �η�k� ���� k� ����� �� ���� �� �Ε�c  � $Z��11����Υ��ƽ����������������������
�޽��ƽ��ν��������8��޵���������cc�����֌RJZ{Rck�������������k{)!k�Z!  Δ��!� k����  k���{  1� � R�� � R�� 1� �� R� �� R��  � $Z��11����Υ��ƽ����������������������
�޽��ƽ��ν��������8��޵���������cc�����֌RJZ{Rck�������������k{)!k�Z!  Ό��!� c����  c���{  )� � J�� � J��  )� �� J� �� J��  � $Z��11����Υ��ƽ����������������������
�޽��ƽ��ν��������8��޵���������cc�����֌RJZ{Rck�������������k{)!k�Z!  ����!� {�����  9Ƅ��{  ���� 1� �ε� 1� ����� s���� �� ���� �� �Ε�q�  � Z��91s�������Ƶ������������������������ƽ��������������2��ޭ{��������R������֔cc{���cJs{�k���֭���s1cRZss1�� ϔ��1� )���c� k�����ڔ��1  R��� �� R�� �� R�Z)���1� R��1� R���  � Z��91s�������Ƶ������������������������ƽ��������������2��ޭ{��������R������֔cc{���cJs{�k���֭���s1cRZss1�� ό��1� !���c�  c�����ڌ��1� J���� �� J�� �� J�R!���1� J��1� J���  � Z��91s�������Ƶ������������������������ƽ��������������2��ޭ{��������R������֔cc{���cJs{�k���֭���s1cRZss1�� ����1� �����c� B֯� �����1  ����� �� �ε� �� ����ε����1  !�����1  !�Ε�2�  � 9��B9s�������Ƶ���������������������������������������������0��֜k��������Z������Δc{skk�sZkk������J�ν{{��kB!�� ϔ�� R�� �� ��� ������έ������֥�������Υ������罭��ﵭܔ��c� ��� �� k����֥�������Ɯ֔ �� k����֥���������{� k��{� k���  � 9��B9s�������Ƶ���������������������������������������������0��֜k��������Z������Δc{skk�sZkk������J�ν{{��kB!�� ό�� J�� ��  {�������ƭ������֥�������Ɯ������罥��ﵥ܌��c� ���� �� c����֜�������Ɣ֌ �� c����֜���������{� c��{� c���  � 9��B9s�������Ƶ���������������������������������������������0��֜k��������Z������Δc{skk�sZkk������J�ν{{��kB!�� ����� 9���� ��  s������������� ������������ �������������c� 1����� �� )����������������� �� )�����������������{  )�����{  )�֕�\�  � 3)��R9Z�cc���������ֵ���������������������������������������/��ƌs����ƽ�{k������Ƅ�ZR��������s1k����B!!� Д��1� {�� �� )�������{BBc{��{{c�BJ{�
���{9 BR{�����11 B��c  {ݔ�� k� �� Ɯ!  1R��	���{Bc��ה �� Ɯ!  1R��Μc{B19���� s���� s���  � 3)��R9Z�cc���������ֵ���������������������������������������/��ƌs����ƽ�{k������Ƅ�ZR��������s1k����B!!� �������������������������
����������������1� s�������� !����� ��������{BBc{��{{c�BJs������������{9 BR{��������������11 B��c  {�������� ��������������� k������������� 	Ɯ!  1R{����� ��������{Bc������������� ������������������	����������� Ɯ!  1R{�������
���Ɯc{B11�������������� k���������������� k������������������������������
������������� ������������������������������������ ��� ��� ��  � 3)��R9Z�cc���������ֵ���������������������������������������/��ƌs����ƽ�{k������Ƅ�ZR��������s1k����B!!� ����1� s���� �� �������{BBc{��{{c�BZ���	���{9 BR{�����11 B��c  ������ ��� �� 	!��!  1Z���	���{Bc���� �� 	!��!  1Z����c{B1R���  !�����  !�֕��� :��cBZ�9Rk���ޭ��ޭ��������������������������ޭν�Ƶ���������.��ƌ{����ε�ss����޽�k{1 ){�sk{��kB9{ZB9 � Дޭ� !���B� Z����� Z�����1  � 1���� �ߔ�)� J� �� � c����  � Jؔ �� � c� Z� 1s�έ� k�έ� k��� :��cBZ�9Rk���ޭ��ޭ��������������������������ޭν�Ƶ���������.��ƌ{����ε�ss����޽�k{1 ){�sk{��kB9{ZB9 � Дޭ� ���B�  R�� �� R�����1  � 1���� �ߔ�)�  B� ��  � Z����  � Bؔ ��  � Z� R� )k�έ� c�έ� c��� :��cBZ�9Rk���ޭ��ޭ��������������������������ޭν�Ƶ���������.��ƌ{����ε�ss����޽�k{1 ){�sk{��kB9{ZB9 � ����� �����B� ������ k������1  !!� R�����  9�� )� ���� �� s� 9������  1  k�� �� s� 9��� s� ������ ������ �֕�� 9{�scckBR{��������������������������ޭ�������ƭֽ�����������+�ֵ�s���Ƶ��ssJ���Ɣc!!s�sJcckR!c1)1� є��R� J���B� 9���)� � ����  1� k��� {ߔ�)� 9� �� )� {������ !)� !{ڔ �� )� {�� R��{� R��{� R��� 9{�scckBR{��������������������������ޭ�������ƭֽ�����������+�ֵ�s���Ƶ��ssJ���Ɣc!!s�sJcckR!c1)1� є��R� B���B�  1���)� ����� )� c��� {ߔ�)� 1� ��  !�  s������ !� {ڔ ��  !�  {��  J��{� J��{� J��� 9{�scckBR{��������������������������ޭ�������ƭֽ�����������+�ֵ�s���Ƶ��ssJ���Ɣc!!s�sJcckR!c1)1� ����R� �����B� s����9� {�9� J������  s���  B��� � B���� 9� )9c����� �� k�Z� {���	���  ���k� {��� �� k�Z� {��� �� �����{� �����{� �Ε�D � +RkZJc�k1BRkks����ޥ����޽��������ֽ�Ƶ����������������������ν�s{��Ƶ��scs�{1� )B�J	B! 9� ќ��� c���B�  !��Z� B�)� !{����B� )��k� ������ � �ݜ� !1Jk�� �� 9�)� )��
��  B��)�  !ڜ �� 9�)� )��R� 1���� R���� R�� � +RkZJc�k1BRkks����ޥ����޽��������ֽ�Ƶ����������������������ν�s{��Ƶ��scs�{1� )B�J	B! 9� є��� Z���B�  ��R� 9�)� {����B� !��k�  ������  � �ޔ �� )Bc�� �� 1�)�  !����� 9��)�  ڔ �� 1�)�  !��R�  )���� J���� J�� � +RkZJc�k1BRkks����ޥ����޽��������ֽ�Ƶ����������������������ν�s{��Ƶ��scs�{1� )B�J	B! 9� ����� )�����B� 9����B� ����9  ������
��B  )����k�  ����� ��9� s��� �� c������� �� ����9� Z�����  Z����)  ��� �� ����9� Z����R� Z����� ������ �Ε�>k)� ;9ck9k�{Z919Z{s���cs�{���ss���Μs��������ֽ�������������������ε�{����ƭ{kR  �  � � � М �� {���B� ����1� Z��1  B����� {����  1����� J�9  !ޜ��� !�� �� 9��s�  ���{� ���{�  Rۜ �� 9��s�  ���� �έ� R�έ� R��k)� ;9ck9k�{Z919Z{s���cs�{���ss���Μs��������ֽ�������������������ε�{����ƭ{kR  �  � � � є��� s���B� {���1� R��1� 9����� s����  )����� B�9  ޔ��� �� �� 1��s�  ����{� ���{�  J۔ �� 1��s�  ����  �έ� J�έ� J��k)� ;9ck9k�{Z919Z{s���cs�{���ss���Μs��������ֽ�������������������ε�{����ƭ{kR  �  � � � ���� )�����B� B�����B� 
Z����1 !�������� ������ ����� c���s!������  ���� �� �����s� 9�����{� �����{� 9��� �� �����s� 9����� Z����� ������ �Ε��� M)c{9c��{R!!)BJZJcZJcJZ{�ZR����kc���sZs��ƭ�έ������������������ֽ�k����Υsc9!� � М�� {���{� �� �� 	���  R����B� 9����� c����� ���9kޜ��� c� �� B����� ������ c���{� �ܜ �� B����� ������ ��{� J��{� J��� M)c{9c��{R!!)BJZJcZJcJZ{�ZR����kc���sZs��ƭ�έ������������������ֽ�k����Υsc9!� � Д�� s���{� �� �� ���� J����B� 1�����  Z�����  ���1cޔ��� Z� �� 9����� {����� Z���{� {ܔ �� 9����� {�����  ��{� B��{� B��� M)c{9c��{R!!)BJZJcZJcJZ{�ZR����kc���sZs��ƭ�έ������������������ֽ�k����Υsc9!� � �� ��� ��� ��� ������������� )������{� {������ �� Z����  s������������B� ������� ��������� ����Ƶ���� ��� �����  )���� �� s���� �� B������� ������ �����{� ������� ���������� �� s���� �� B������� �����{� ���� ������{� ����������� ������������������� ��� ������������ �������������������� !R{BR���s119�)91B!!1)9kBBJ�{��RZs��{c���ƽ������������������{k���ν�sZ)!� М�B� {��ƽ� )��{  �ޜ !���� 9��s� 9����� 1לΜ� {� �� !�� �� ����� c�{c� cܜ �� !�� �� ������ 1��{� Z��{� Z��� !R{BR���s119�)91B!!1)9kBBJ�{��RZs��{c���ƽ������������������{k���ν�sZ)!� Д�B� s��ƽ� !��{� �֜ ���� 1��s� 1�����  )הΜ� s� ��  �� �� ����� c�{c� Zܔ ��  �� �� ������  )��{� J��{� J��� !R{BR���s119�)91B!!1)9kBBJ�{��RZs��{c���ƽ������������������{k���ν�sZ)!� ��������� ����������������������B� )�������� ��������������{  B���� J������������������ 9���s� ����������� {��������������������� ���������	����  )����� ��� ��� ������ ���� �� Z�������������� Z��{c� 	!��������������������������������������� ����� �� Z��������������� c��������������{� ������ ����{� ���������������������������������������������������������������������������o�  !JsB9��ƽs1! !B)!!)B1)R��kJ19RZ1Jks�������������������ss���έ�kR� М �� {���� R� �� ��R!���{� R����� )ל�{� {���� 1���!  ���B� sݜ��� 1���!  ���� �έ� Z�ƭ� Z���  !JsB9��ƽs1! !B)!!)B1)R��kJ19RZ1Jks�������������������ss���έ�kR� ����� ��� ��� �������� ������������������ s�������� B��� ���������� ��R{�����������{� B�� �������� !�������������������� ��� ���������{� s���������������� )����!� �����B� k������������� ����������������  )���!�  ��������������� ���	��������ƭ� R��� ��� ���ƭ� R�������	����������������� ��� ����������������������������� ���������  !JsB9��ƽs1! !B)!!)B1)R��kJ19RZ1Jks�������������������ss���έ�kR�  ��� ������������ ��� ���������� ��� �� ������������� 
���������������  ���Z������{� 
��������������� Z���������������������� �����
�����{  )���� ��� ��� �� {����!  Z���� �����B� B���� ���	����������� {����!  Z����� ������ Z�������� ���������������� ������������� �������������������������������������� ��������� ���������� LRkk1{���քR 19BJJs�{J)ZZB9���{sR911)!)1Z{c�����������������ֵc����Όkc9� М�c� k���B� c��c� s� 1c� �� ��k��֭� ל�B� {������� )����  ���s� ��ݜ��� )����  ���� �ތ� Z�ތ� Z��� LRkk1{���քR 19BJJs�{J)ZZB9���{sR911)!)1Z{c�����������������ֵc����Όkc9� М�c� c���B� Z��c� k� 1Z� �� c��֭�  ל�B� s����  !����  �� s� �ݜ���  !����  ����  �ތ� R�ތ� R��� LRkk1{���քR 19BJJs�{J)ZZB9���{sR911)!)1Z{c�����������������ֵc����Όkc9� �� c� �����B� )����
�c  !�� R���� �� �Z c��Z�������� Z����B  )������� {����  9��� s�  �Zs��{Z�������� {����  9��� �� ������� ������� �Ε��� :Rs�9c����ƜJ!ZZcRc����Z��s{�����kcJ1Jkk��޽�����������ƜR���ޭZcR� Ϝ �� R���B� {��{� !���1� 1�Zs��sc���s� ל�B� {��Ɣ���� )�� �� ��� �� J�Z{��csܜ��� )�� �� ������ 9��c� Z��c� Z��� :Rs�9c����ƜJ!ZZcRc����Z��s{�����kcJ1Jkk��޽�����������ƜR���ޭZcR� Ϝ ��  J���B� s��{� ���1� )�Rk{�kZ���s�  ל�B� s�޽�����  !�� �� {�� �� 9�Rs��Zkܜ���  !�� �� {�����  )��c� R��c� R��� :Rs�9c����ƜJ!ZZcRc����Z��s{�����kcJ1Jkk��޽�����������ƜR���ޭZcR� �� �� �����B� Z�����{� ������1� c������������s� �����B� ���������� {��� �� )���� �� �������������� {��� �� )���� �� �����c� �����c� �Ε��� 99c�99�����ΥJ1){cks��k�������������sZ  B{���޵�����������ΌZ����{1R)� ������������������������������������� ����� 9������1� �������������� B��������� ������B� !������Z���������Μ� B�������������� ������������ ����������s� Z��BJ���������������� )������ {�� ������� B�����J������������������������ ��������� )������ {�������������� 9������������R� Z���������������R� Z������������������������Ƶ������������� ��������������������������� ���� 99c�99�����ΥJ1){cks��k�������������sZ  B{���޵�����������ΌZ����{1R)� Ϝ��  )���1� ���� 1��B� ������R���Μ� 1ל�s� R��BJ������  !����� s�� �� 1�����Jۜ���  !����� s�����  )��R�  R��R�  R����ƭ��ƥΜ� 99c�99�����ΥJ1){cks��k�������������sZ  B{���޵�����������ΌZ����{1R)� ���� Z����1� ���� �� �����B� �������k������ �����s� ����Js����� Z��� �� )���� �� ������c���� Z��� �� )������� J����R� �����R� �������� ��� ���� !k�k!������֥9J���#��ƭ���������ƌcJ  1ks�����������������ks���Z1!�  Υ k� ����� J� �� Z��Z� 9���� 9c����{� Zץ��� Z�� )��ֽ�  ����� {���!� Z����  J{ݥֽ�  ����� {��֜� ���{� 9��{� 9������������ϥ� !k�k!������֥9J���#��ƭ���������ƌcJ  1ks�����������������ks���Z1!�  Μ k�  ����� B� �� R��Z�  )���� 9Z����{� Rל��� R�� )��ν�  ����� s���!� R����  Bsݜν�  ����� s��֜� ���{�  )��{�  )��������������Μ� !k�k!������֥9J���#��ƭ���������ƌcJ  1ks�����������������ks���Z1!�  �� �� B������ c���� �� 9���� Z� ������! s����{� ������ ���� B������ 9������ �����!� ������  ������� 9������ ������� )�����{� {����{� {��������������������8  � 8Js�JJ������1Z�������s������������RJ�����֭�����������
���k���Δ)� ̥� ����  1���R� ���  9����{ ����� Z֥ �� Zֽ ����{� ���ֽ� c�� �� Z����9 Bݥ�{� ���ֽ� c���s� {����s� 1���s� 1����{  9�����B R��� kХ  � 8Js�JJ������1Z�������s������������RJ�����֭�����������
���k���Δ)� ̜� ����� !���R� ����  )����{ ����� R֜ �� Rν ����{� ���ν� Z�� ��  R����9 1ݜ�{� ���ν� Z���s� {����s�  !���s�  !����{  9�����B J��� cМ  � 8Js�JJ������1Z�������s������������RJ�����֭�����������
���k���Δ)� �� �� ������  B������R� ������  c�����{ !������� ���� �� ���� 1�����{� ������� ���� �� ������B J�����{� ������� �����s� )������s� s�����s� s����{  ������B ����� )���C � 71Z��)�������!k�������{�����������R1R������｜����������	��{��ֵZ� � ̥ k� J�������1 ����� B�� �� 
)�����  k����c� Z֥ �� R�{ {���s� R���{� c���Z� 
R����c �ݥ�s� R���{� c�����!� {������  !������  !����� 1���֜�  J���c� kѥ � 71Z��)�������!k�������{�����������R1R������｜����������	��{��ֵZ� � ̜ k� B�������1 ����� 9� �� 
�����  c����c�  R֜ �� J�{ {���s� J���{� Z���Z� 
B����c  �ݜ�s� J���{� Z�����!�  {������  ������  ����� !���Μ�  B���c�  cќ � 71Z��)�������!k�������{�����������R1R������｜����������	��{��ֵZ� � �� {� !��������9 k����� �� !�����  ������  ������c� ���� �� s��{ R�����s� J����{� ����Z  )�����c ������s� J����{� ������!� {������� ������� ������ ������� )����c� Z����  � 1��B9�����s������� �����s{���{9!)R{�������ƥ�����������s��޵�)� ˥ J� ��ν޽1 R1�� �� s���B� B{{R  9��ƽ�  !֥�Z� R  �����!� ���� k� � c{{1  Zݥ��!� ���� k����c� 1����)�  c���)�  c����� Z���{� ����B� ѥ  � 1��B9�����s������� �����s{���{9!)R{�������ƥ�����������s��޵�)� �� ��� ������������ �����������������B� ��ν޽) J!����������������� s���������������9� B{{R  )����� ������������������ ����Z� 	R  ������������!� ���� c������� ������� c{{1  R����� ��������������!� ����  c����c� )��������)� Z�����������)� Z������������� R���������{� {���������B�  �� ��� �����������������������  � 1��B9�����s������� �����s{���{9!)R{�������ƥ�����������s��޵�)� �� c� B��ν޽9 ������ �� 9����J  !k{{R  R������� c��
Z  RR  �������!� ����� J����  9s{{1  {������!� ����� J����c� �����)� !����)� !��� �� �����{� �����B� B����� s�s����,�) ���Υ���������{����{1B{���������Υ���޽�������Z���{1� !ʥ{� B1���R  ���J� ���s)� 9�ץ�� B�� k� J��R� ���)� 1ۥ k� J��R� ��� �� B��91� {��91� {���� J�� �� k�� c� �ҥ� s�s����,�) ���Υ���������{����{1B{���������Υ���޽�������Z���{1� !ʥs� 9!���R� ���J� ���k)� )�֥ � 9�� k� B��R� ���)� !ۥ c� B��R� ��� �� 9��91� s��91� s�� � B�� �� c�� c� �ҥ� s�s����,�) ���Υ���������{����{1B{���������Υ���޽�������Z���{1� !����{)� �������Z  {�����{)� B������91� !9������ B���� �� ����s! !B�����Z!� s���� �� ����s! !B���� �� !����	RZB)))Z����	RZB)))Z�����  ���� �� 9���� k� c������ 4J��JZ�����1 s��֭���������k����s)s�{s�������ޭ���ֵ����焌���!� �!!ȥ�{B�B���!1�	�J! ��B9!!1{֥�! �� {�B���B9��	kB 1�ڥ {�B���B9����1� k���BZ99!19����BZ99!19���s c�R ��)ѥ� 4J��JZ�����1 s��֭���������k����s)s�{s�������ޭ���ֵ����焌���!� �!!ȥ{s9� 9���! !�{B� ��9)!s֥�!� ��{� 9��{9)��k9� !�ڥ{� 9��{9)����)� k��{9R))!)���{9R))!)���k  Z�R  ��)  ѥ� 4J��JZ�����1 s��֭���������k����s)s�{s�������ޭ���ֵ����焌���!� �!!��
�֜{cZZs������1�����	�ƽ�9Z�������ƽ��������J)cc�������s���{�����ƽ�ƽ����ƽ{9c������s���{�����ƽ�ƽ���Ɣ�������������ƽ���������ƽ������1){����s19�����B)B������ 3���)�����1 9�ν���ƭ��s��k�޽�11sccs�����������έ����k���� �!��������������������� �����������
���������{{�Z	������������ ��� ���	�RR��������� ��������ZJs��������������� �� ���������c9Jss�����	����s���s�������������� �������JR{�� ������������������������s���s��� ��� �������������� ��� ������� ������������������������B9J�������������99Z��������{99{�� ��� ������������������������� 3���)�����1 9�ν���ƭ��s��k�޽�11sccs�����������έ����k���� � !ť�ss�R ��JJ��RBk���Z1Bkk���k��{k���BJsץ�k��{k��{{����ť91B���)1R�s)1sѥ� 3���)�����1 9�ν���ƭ��s��k�޽�11sccs�����������έ����k���� � !������� ���������ֻ���������������������������������������������������� �� 2R��Z�����B )��ƽ����c�ss{k���c!RscZ�������������έ������!� !���������� 2R��Z�����B )��ƽ����c�ss{k���c!RscZ�������������έ������!� !���������� 2R��Z�����B )��ƽ����c�ss{k���c!RscZ�������������έ������!� !������ �� 2!�΄�����k ���ε������{�s�ބZ1ZkBc������������ｽ����{��1� �!!��������� 2!�΄�����k ���ε������{�s�ބZ1ZkBc������������ｽ����{��1� �!!��������� 2!�΄�����k ���ε������{�s�ބZ1ZkBc������������ｽ����{��1� �!!����� �� 1sƭ�����{ ��޽�ε����{ss��sR1cRR���祭�������������ތ��9� �!� ��������� 1sƭ�����{ ��޽�ε����{ss��sR1cRR���祭�������������ތ��9� �!� ��������� 1sƭ�����{ ��޽�ε����{ss��sR1cRR���祭�������������ތ��9� �!� �����i� �� )��k���� s�����{��s�ss��Z1BRZ����ε��������Zk� �)1)!��������� �� )��k���� s�����{��s�ss��Z1BRZ����ε��������Zk� �
)1)!��������������������� ��� ����������� ��������� ������� �������������������������� �������� ��� ��� ��� ������� ���������������������� ��� ������������������ �������������������� ��� ������������������������������������������ ������������������ ������������������������������������ ��� ������������������ ��� ���������������������� ������������� ���� �� )��k���� s�����{��s�ss��Z1BRZ����ε��������Zk� �)1)!������  ��� %B�c����  c���έ�{��k�{��{99cR��ֽ������ ���ޥ�����J!� !!)!)���������  ��� %B�c����  c���έ�{��k�{��{99cR��ֽ������ ���ޥ�����J!� !!)!)���������  ��� %B�c����  c���έ�{��k�{��{99cR��ֽ������ ���ޥ�����J!� !!)!)����� �� $�k{���  J���ƽ�s��s{s��{9Rs{��ֽ�������֜����{� !�)!!)�������� �� $�k{���  J���ƽ�s��s{s��{9Rs{��ֽ�������֜����{� !�)!!)�������� �� $�k{���  J���ƽ�s��s{s��{9Rs{��ֽ�������֜����{� !�)!!)�����!)!�� $Z�1��ޭ !�����sk��sks��c)Z�������������������1� !!)!��������!)!�� $Z�1��ޭ !�����sk��sks��c)Z�������������������1� !!)!��������!)!�� $Z�1��ޭ !�����sk��sks��c)Z�������������������1� !!)!������)1)!��� �!{�ֵ! R��Ƶkc��ks���B1���������֔����!� ��))���������������������������� �� ��� �� ��� ������� �������������������������������� ��� ������������������ ��� ���������� ��� ��� �� ��� ��� �������� �� ��� �� ��� ��� ��)1)!��� �!{�ֵ! R��Ƶkc��ks���B1���������֔����!� ��))���������)1)!��� �!{�ֵ! R��Ƶkc��ks���B1���������֔����!� ��))������9B))!��� !ZB1�ƽc  ){���{{��k���Z9s�����������֜����1� �
!!))!��������9B))!��� !ZB1�ƽc  ){���{{��k���Z9s�����������֜����1� �
!!))!��������9B))!��� !ZB1�ƽc  ){���{{��k���Z9s�����������֜����1� �
!!))!�����11�)!!� !)c{ƽ�  R��֜�������RZ������������	�{����! � �!�)9!��������11�)!!� !)c{ƽ�  R��֜�������RZ������������	�{����! � �!�)9!��������11�)!!� !)c{ƽ�  R��֜�������RZ������������	�{����! � �!�)9!�����j1)!�)�! � � BB���9  1{�Δ{�����{J�����������眵���)� �!1�!!1��������1)!�)�! � � BB���9  1{�Δ{�����{J�����������眵���)� �!1�!!1����������� ��������	������������ �������� ��� ��� ���
������������� ��� ��� ������������������������� ��� ��� ���������������������� ������������������������������� ������������������������� ��� ��� �������������������� ��������������������������������� ��������� ��������������������� ������������1)!�)�! � � BB���9  1{�Δ{�����{J�����������眵���)� �!1�!!1�� ��� ��� ��� ��� ���� ��� ��� ��� �� ��� ��u 1�)1)�!� �c�Ɯ c��sk����kBk������������ֵ���� ))11�! ���������� 1�)1)�!� �c�Ɯ c��sk����kBk������������ֵ���� ))11�! ���������� 1�)1)�!� �c�Ɯ c��sk����kBk������������ֵ���� ))11�! ��������������������� ��� ����������� ��� ����������������� ���������� �������������������������������������� ������������������������������������������������� �������� ����������������� ����������������� ��������� ����������������� ��� ���������������� ���
���������������������������������������������������������������������������������������� �����P�1 !�� c��R  9��k�����B9�������������ք!�  �))!!����������1 !�� c��R  9��k�����B9�������������ք!�  �))!!����������1 !�� c��R  9��k�����B9�������������ք!�  �))!!������������� ��� ����������������� �������������������������� �������������������� ��� ��������������������������������������������������������������� ������������ ���������������������� ��������������� ��� ����������� ������� �������������������������� �������������������� ������������������������ �����������������199)�1�! �� 	)k��! Zs{���c9���������� ����ֵ�1� !!)!!)!��������199)�1�! �� 	)k��! Zs{���c9���������� ����ֵ�1� !!)!!)!��������199)�1�! �� 	)k��! Zs{���c9���������� ����ֵ�1� !!)!!)!������ 9�1911))!!�� 1{�R  )Jkc��c9Z���������� �����s� !)�!	!!))!!� �����������������������������������������������	������������������������������� ���������������������	�������������������������������������������������������������������������������������������������������� �������������������������������������������������������� ��� ��� ���������������������������������������������������������������������������������� ����� ���������������������������������9�1911))!!�� 1{�R  )Jkc��c9Z���������� �����s� !)�!	!!))!!�  �������� 9�1911))!!�� 1{�R  )Jkc��c9Z���������� �����s� !)�!	!!))!!�  ����� �919�1�)!� R��)  )JJZR!J����������������R� �
!!!)!!� ��������919�1�)!� R��)  )JJZR!J����������������R� �
!!!)!!� ��������919�1�)!� R��)  )JJZR!J����������������R� �
!!!)!!� ����� � B�1911�)!�� !c�c !))!c��������������ޥR�  �!)!�!��������� B�1911�)!�� !c�c !))!c��������������ޥR�  �!)!�!��������� B�1911�)!�� !c�c !))!c��������������ޥR�  �!)!�!������� B�91))11)� �� csZ)� s��������������B� )!) ))!)!�  �������� B�91))11)� �� csZ)� s��������������B� )!) ))!)!�  ������� ��� ���������� ��� ��� ��� ������� ��� ��� ��� ��� ��� �������������������������� ��� �������� ������������������ ��� ������������ ��� ��� ����������������� ��� ��� ��� �� ��� ��� �� �������������� ��� ������������� B�91))11)� �� csZ)� s��������������B� )!) ))!)!�  ����� �B9B�9�1991!�� 9Rk{s{������������������J� �)�!!!!)�  ��������B9B�9�1991!�� 9Rk{s{������������������J� �)�!!!!)�  ��������B9B�9�1991!�� 9Rk{s{������������������J� �)�!!!!)�  ����� �B9B99�1BB9�!� 1c{������������������֜!� �!!))!)!� ��������B9B99�1BB9�!� 1c{������������������֜!� �!!))!)!� ��������B9B99�1BB9�!� 1c{������������������֜!� �!!))!)!� ����� �9BJBB�1B91))!� 	Bk{��������
���������Z� !!))!!))9)�� ��������9BJBB�1B91))!� 	Bk{��������
���������Z� !!))!!))9)�� ��������9BJBB�1B91))!� 	Bk{��������
���������Z� !!))!!))9)�� �����BJB9JB91�9))!� 	9Jc��������������s)� !99)!)� �  � �� �� �� �˽ ƹ� ��� �ٽ �ٽ �ս �� Ƣ�BJB9JB91�9))!� 	9Jc��������������s)� !99)!)� �  ��������BJB9JB91�9))!� 	9Jc��������������s)� !99)!)� �  ����� �JRB9RJ9BJJB)!� 1c�{{��������1� �)
!!1��  �ƁƁƔ�JRB9RJ9BJJB)!� 1c�{{��������1� �)
!!1��  ��������JRB9RJ9BJJB)!� 1c�{{��������1� �)
!!1��  ����� �ZZRJJB99JJ9� 	1BBZk{{�c�R�  ��)9!))!!�� �ƁƁƔ�ZZRJJB99JJ9� 	1BBZk{{�c�R�  ��)9!))!!�� ��������ZZRJJB99JJ9� 	1BBZk{{�c�R�  ��)9!))!!�� ������ Z�J	99BRJ � !)99�)� �!!)!1)))11!!  � �ƁƁƔ� Z�J	99BRJ � !)99�)� �!!)!1)))11!!  � 
ƽ�ƽƽ���� ��� ��� ���ƽ�����ƽ�ƽƽ���� ��� ����ƽ�����ƽ��� ��� �� ��� ��� ��ƽ��� ��ƽ��� �������ƽ������� ��� �� ����������� ��� ��� ��� ��� ���ƽ����� ��� ����ƽ��������ƽ��� ��� ��� ������� ���ƽƽ�Z�J	99BRJ � !)99�)� �!!)!1)))11!!  � ����� �	RJBJJBJB!� �  �  �!� !1)!11!)1!1!)!!    �ƁƁƔ�	RJBJJBJB!� �  �  �!� !1)!11!)1!1!)!!    �ƁƁƔ�	RJBJJBJB!� �  �  �!� !1)!11!)1!1!)!!    ����� ��JRR9�� �� cƜ!� )�����!�  ��)!!1BJ!!))!    � �ƁƁƔ��JRR9�� �� cƜ!� )�����!�  ��)!!1BJ!!))!    � �ƁƁƔ��JRR9�� �� cƜ!� )�����!�  ��)!!1BJ!!))!    � ������BJRJB!��  !�����J� Js��9� �))1)!11!�  ��Ƃ�{ 9ck�� ��� ���	�J  1Jk�����	�J  1Jk������R!  1cR�1Rc���
�{ 1c�������Ɣ��cc����k1  c��BJRJB!��  !�����J� Js��9� �))1)!11!�  ��Ƃ�{ 9ck�� ��� ���	�J  1Jk�����	�J  1Jk������R!  1cR�1Rc���
�{ 1c�������Ɣ��cc����k1  c��BJRJB!��  !�����J� Js��9� �))1)!11!�  ����� Js��� ��� ���	�Z  9Z������	�Z  9Z�������k)  9sk�9ks���
Δ 9s�������ﵵ�ss���ބ9  s���BJZ9��  k����)� )!�  �)))!!)9)!)1��  �!�Ƅ��!�  s�� J� s���J� {���J� {���s1�  ����� J����� 1��ƥJ� 1���BJZ9��  k����)� )!�  �)))!!)9)!)1��  �!�Ƅ��!�  k�� J� k���J� {���J� {���k1�  ����� J����� 1��ƜJ� 1���BJZ9��  k����)� )!�  �)))!!)9)!)1��  �!����)�  ��� Z� ����Z� ����Z� ���ބ9�  ����� Z����� 9����Z� 9����JZB��  ����R9!  J!� !!))!9�!�1)�)1!!�� !�������������� ��� ��������������������������������������������������������������������� ������� ��� ������������������� �������������������������� ����������������	�������Δ1� Z�ƭ9� !��� ����!� {�������������ν!� {���������R� 	������������ {� J�������9� !� !c����������������� ������� ���������JZB��  ����R9!  J!� !!))!9�!�1)�)1!!�� !�Ɔ��1� R�ƭ9�  !���!� {���!� {�� R�  ��� {� J��� 9� !� !c��JZB��  ����R9!  J!� !!))!9�!�1)�)1!!�� !����9� k���J�  )���)� ����)� ��� k�  ��� �� Z��� J� )� )s���9)��  �  ����R! �9�!� !)!))1)!!))�� !!�Έ��1�  {���!� !� !����!� !� !��� �� c��� s� �  {����  ���9)��  �  ����R! �9�!� !)!))1)!!))�� !!�ƈ��1�  {���!� � !����!� � !��� �� Z��� k� �  {����  ���9)��  �  ����R! �9�!� !)!))1)!!))�� !!����9�  ����)� JZB� )����)� JZB� )��� �� ���� �� )ZZ�  �����  ���y ��  � 	 c�k �1)� �!)!!11!!!�  �!))�Έ� 9� ��  9���� � �  s���� � �  s�� !� � ��� J�  � �  ���!�  ��� ��  � 	 c�k �1)� �!)!!11!!!�  �!))����������� ������� ������������� ���������� ��� ��� ��� ������������� ��� ��� ��� ����������������� ��� ��� �������9�  �  9��Υ� �� s��������� ��  s�� !�  � ��� J� ��  �����ƭ�  ������������� ��� ��� ��� ����� ��  � 	 c�k �1)� �!)!!11!!!�  �!))��� J� !ZB�  k���� ����J�  ����� ����J�  ��� )� )��{� ����� Z�  c��s!�  ���)�  �����   � !�s  �� !!1!)1!)!�� )1))�Ή� {�  � � 1���� � �  ���� � �  �� ��  �� ��� c�  ��  s�� J� 1s���   � !�s  �� !!1!)1!)!�� )1))�Ή� {� ��  )���� ��  ���� ��  �� �� �� ��� c� ��  k�� B� )k���   � !�s  �� !!1!)1!)!�� )1))��� ��  B���B� Z����� !��� s�  ���� !��� s�  �� �� s�� �� !����� s�  c���!�  ����� ������� ��  � Z� �� !1)!!!!�	))�� �)�Ή� � �� ��� Z� �!ZsZ� ��� Z� �!ZsZ� ��� ��  �� � ����� �1ss)�  k����  � ����� ��  � Z� �� !1)!!!!�	))�� �)�Ή� � �� ��� Z� �RkR� ��� Z� �RkR� ��� �� �� ����� �)kk!�  k���� ����� ��  � Z� �� !1)!!!!�	))�� �)��� �  ����� c���� k�  ������ε!�  J�� k�  ������ε!�  J�� �� {��� �������  )����ֽ{� ����s� )Z!� !�������  �� 
!�Z � � !1))9)!!1)!))!�!�  �!!1)�Ί� �� �9sZ� s�� �� �)����B�  J�� �� �)����B�  J�� �� �B���  ��� !� � {�� �� �� �� � � ����  �� 
!�Z � � !1))9)!!1)!))!�!�  �!!1)�Ί� �� �)kR� k�� �� �!����9�  J�� �� �!����9�  J�� �� �9��� ��� !� � s�� �� �� �� �� ����  �� 
!�Z � � !1))9)!!1)!))!�!�  �!!1)�������������������� ��� ��������� ��� ��� ���������������� ���������� ������������������ �������� ��� �������� ������������� �� ����εZ� ����� ��  )���������  ������  )�� ��� ��  ��� �� !�������  Z�������)�  ���������� ���� � !��� J� B����� �������������������������������Q�    � 	)) c� � � �)!11!!)!!))B9)  �!1)1)�Ί� �� Z���R� s�� 1� �9��� ��  ��� 1� �9��� �� ��Μ� �)��Υ� � 9�� ��  � {�� R�  B�� 1� � � ���    � 	)) c� � � �)!11!!)!!))B9)  �!1)1)�Ί� �� R���J� k�� 1� �)��� ��  ��� 1� �)��� �� ��Μ� �!��Υ� � )�� �� � s�� R�  B�� 1� �� ���    � 	)) c� � � �)!11!!)!!))B9)  �!1)1)������������������������������������
���������������������������������������������������������������������	��������������������	�������������������������������������������������������
�������������������������� �������� s������9� ���������� ����� 9� 
������������ ����� ��������� !������ ��  B���������k� k�����9�  ���{� {����������������	������������
�������������������  �  � �! kR � � !)11!B!)!B1)9B!99!1�  �!11!�������������� �������� ������������� ��� �������������� ���������������������������� ���	������������ ��� ��� ����������������������� ������������������������������� ��� ��� ���������1� �����΄�   �{�֜� ��������1�  )���֜� � ������1� )�έ�  �!�����c� � ������1� B�����֜� �������1� �Bs9� 9�� ���	������������ ����������������  �  � �! kR � � !)11!B!)!B1)9B!99!1�  �!11!�Ί� 1� ��� {� �s�Μ� � ��� 1�  )�� �� � ��� 1� )�Υ� ���� c� � ��� 1� 9�� ��  ��� 1� �9k)� )���  �  � �! kR � � !)11!B!)!B1)9B!99!1�  �!11!���� ��� ��� ��� ��� ��� ������� ��� ������������������������������ ������9� ����� �� J{9B��������  �� ��� 9�  c�� ��  �� ��� 9� c����  Z������ {� ��� ��� 9� c����� ��  ��� 9�  !����Ɣ!� 9������������ ���? �� !)!9��  )1)!!1)1!!)J91!B)))1�! �
  ))�֋� �� ���  �!���!� B�� 1�  ��� !� B�� 1� ���  �9��� �  � s�� �� � ��� ��  !�� B� 9���k� Z�� �� !)!9��  )1)!!1)1!!)J91!B)))1�! �
  ))�΋� �� ��� ����!� 9�� 1�  ��� !� 9�� 1� ���  �)��� � � k�� �� ��� ��  �� B� )���c� � R�� �� !)!9��  )1)!!1)1!!)J91!B)))1�! �
  ))���� �� �����  s������)� )����� 9� )��� )� )����� 9� )��� !������ �  B�� ��� �� ������ ��  ��� J� Z������ε�Zc�� ������ !))! �!� )!!)1!!)99!�)!19�  �!) �֋� �� �� �� �!��֭� ����� !��֭� ����� !�֔R�!��� �� �9��� B� Z�� k� s�� k� ����!� ����1 B����� !))! �!� )!!)1!!)99!�)!19�  �!) ��� ����������� ��� ��� ����������� �������������� ��� ��� ������� ������������ ��� ��� ��� ��� ��� �� ����� �� ���έ� ������ ��έ� ����� �ΌJ���� �� �)��� B� R����c� k��� k� ����� ����1 B�������������������� !))! �!� )!!)1!!)99!�)!19�  �!) ���� �� ���� ��  ��������� !������� )������ !������� 
)����s������� �� ������� J� {���� {� ���� {� {�����޽�� ����9 J���y�� �	!  � ,!!)B9)1)))1)!1!!1!))  !)  �֋� �� �� s� !k�R)Z��� s� !���)� s��s� !���)� s�֭� s�� s� !��� � ����k� �� k� 9���9� R���� B��J ����� �	!  � ,!!)B9)1)))1)!1!!1!))  !)  �֋� �� �� s� c�B!R��� s� ���!� k��s� ���!� k�֭� k�� s� ��� � {���k�  �� k� )���)� B���� B��J ����� �	!  � ,!!)B9)1)))1)!1!!1!))  !)  ���� �� c���� �� �����ν���� �� �����)� )������ �����)� )������� ��� �� {������ � J�����{� ���� {� Z�������� ����� J��Z ���L��  �� !1!!!  !� )!11))B!1)!!11J))!� !)� �֊� � ��� k� ��� � ��ƄB1� B��� ��ƄB1� B��)9�� 1�  � Z�� �� c�֭Z11� ��� k� s���s��� �� �����  �� !1!!!  !� )!11))B!1)!!11J))!� !)� �֊� � ��� k� ��� � ��ƄB1� 9��� ��ƄB1� 9��!)�� 1� � R�� �� c�֭Z11� ��� k� k���k{�� �� �����  �� !1!!!  !� )!11))B!1)!!11J))!� !)� ���� � ����� {� ����� � J���J9� 9����� J���J9� 9��������� 9�  B�� ��� �� !����k99� ����� {� Z��������� �� ����� � !�)!!  � �)91)919!!)11!))1))!�!�  �֊� k� )�� k� ��� �� B֔� B���s��� ��  �!��� )� ��� k� ��� � BJ1�  ���� � !�)!!  � �)91)919!!)11!))1))!�!�  �֊� k� !�� k� ��� �� 9֔� 9���k��� �� ���� )� ��� k� ��� � BJ)�  ���� � !�)!!  � �)91)919!!)11!))1))!�!�  ���� {� ����� {� B����� �� c����� c��������� �� {������ )� ����� {� J���� � J{��B�  ����� �   � 	!91!))!� �!119)9JB))�19!) � ��֊� �� ��� k� ��� k� B�k� B���1� � ���� ��� k� s���  ��!�� Z���� �   � 	!91!))!� �!119)9JB))�19!) � ��֊� �� {�� k� �� k� 9�k� 9���1� � ���� ��� k� k���  ���� R���� �   � 	!91!))!� �!119)9JB))�19!) � ����� �� ����� {� ����� {� ����{� ������9�  ��� ���� B����� {� )�����  ���� 9� ε���
�  �� !�19)� !!1!))!!R1!1)1B�1))1)�      �։� � ��� k� ��� k�  � R�k�  � R���� � Z���  � � ��� k� s��k  1��� !����  �� !�19)� !!1!))!!R1!1)1B�1))1)�      �։� � ��� k� ��� k�  � J�k�  � J���� � R���  � ��� k� k��k  1��� ����  �� !�19)� !!1!))!!R1!1)1B�1))1)�      ���� � !���� {� ���� {� ))� B����{� ))� B�������  )�� ���� )!� ������ {� )����{  9����!� J������t�  �� �!� )!JR91))1Z!!19)�1!!91))!� �    ��������� ������� ��������� ��� ������� ������� ��� �� ��� R� Z���� ����޽� ��� ��� �� !��� Z� ���!�����B� k��k  �R���� � ��������  �� �!� )!JR91))1Z!!19)�1!!91))!� �    �։� �� �� R� R���� �� �{ֽ� �� � {�� �� ��� Z� �� �� B� c��k  �J{��� � ����  �� �!� )!JR91))1Z!!19)�1!!91))!� �    ���� ��  s�� Z� s�������  !�Zs����cc������  !�Zs����cc�� ��� �� )������ k�  J�Z ����{Z������ J� )����{  �������  ��� ���^��� � !�)!))� �!)!99ZB)19R!BB11JR9))11!1�     �����������  ��� ��  � ��� k� ����k� � ����)� ����� � Z�� �� B�� k� ���)  � )����� � !�)!))� �!)!99ZB)19R!BB11JR9))11!1�     ����������� ��� ��� ��� �������� ��� �����֔���� ��� ��� ��� ���� ����� �� � ��� k� ����k� � ����)� ���� ����� � R�� �� 9�� k� ����)  � !�� ������ � !�)!))� �!)!99ZB)19R!BB11JR9))11!1�     ����������� 9��� ��  c�� ��� {�  �����{�  �� ����)� Z�������  9� ��� �� J���� {� {���)  �� ���~�� � �!B91))� )!)11�! 9�J1991!!JRB1BB11!)�    �������  R���B��c) ���� 9�� R�  ��� ��  �BB!� Z�� ��  �BB!� Z�� )� B�� 1� �)B9�!��� �� B�� k� R��ތ� � k���� � �!B91))� )!)11�! 9�J1991!!JRB1BB11!)�    �������  R���B��Z) ����  1�� R� ��� �� �99� R�� �� �99� R�� )� 9�� 1� �!91���� �� 9�� k� R��ބ� � c���� � �!B91))� )!)11�! 9�J1991!!JRB1BB11!)�    �������  Z���J��k) ���� B��� Z�  B���� ��  s���ƽ�� ��� ��  s���ƽ�� ��� )� ����� 9� !������������ �� s���� {� Z����� �� ����� �  �� 
1!)1J1)!� )1�9�)JBRBBJB99Bk9)!1B99)�  ������ 1) B��k ����  ��� �� ��ck� � ���� � 9�{���������� � 9�{��������� �� � B�� k�  � k�{ ���֜{��� �� B�� �� � � ���� �  �� 
1!)1J1)!� )1�9�)JBRBBJB99Bk9)!1B99)�  ������ 1) B��k ����  ��� �� ��ck�  � ���� � 1�s�����{{��� � 1�s�����{{�� �� 9�� k� � c�s ���Μs��� �� 9�� �� � � ���� �  �� 
1!)1J1)!� )1�9�)JBRBBJB99Bk9)!1B99)�  ������ 9) J��{ ����  ��� �� s�ޥk{� 1c�� ���� �� ��� ��������� �� ������������ �� ������ {�  Z���� �������� �� ����� �� � ������P���  �)9B)� �"11))!!)B9)9RJ1)1)9B))1!J9!!!!� �����{  �Ό���R ����  ��� R� 
RΥ{ s� ���� � ����1c��� � ����1c���� � {�� k� R���R1��� � k�� � �����  �)9B)� �"11))!!)B9)9RJ1)1)9B))1!J9!!!!� �����{  �Ό���R ����  ��� R� 	JΥ{ s� {��� � ����1c��� � ����1c���� � s�� k� J���R1��� � c�� � �����  �)9B)� �"11))!!)B9)9RJ1)1)9B))1!J9!!!!� ������  �����Z ����  ��� Z� 	J���経 �� ����  �� ����9k���  �� ����9k����  !�� ��� {� s������Z9��� � )����� � c����� ��
  ))9JBB9� � $!1)91B1!)9B)9B1)1919))B1!)9)!)!))��  � ����{k����k��֌k���{9R����k�����11Rƥ���{99�Ɯ���{k����B1c��k���Μk�����  1k�ޜkk��RR���ƌ{�����{k���{k���c1c��� �� ){��  �֌k���ƌ{���� ��  � c���Z� c��� ��� R�  ���� ��� R�  ����)� �)������� k� 9����  )�� 1� ��� �� �� ��
  ))9JBB9� � $!1)91B1!)9B)9B1)1919))B1!)9)!)!))��  � ����{k����k��Όk���{1R����k�����11Rƥ���{11�Ɣ���{k����B1c��k���Δk�����  1k�ޔkk��RR���ƌ{�����{k���{k���c1c��� �� ){��  �Όk���ƌ{���� �� � Z���R� Z��� ��� R�  ���� ��� R�  ����)� �!������� k� 1����  )�� 1� ��� �� �� ��
  ))9JBB9� � $!1)91B1!)9B)9B1)1919))B1!)9)!)!))��  � ����{����{���{����9Z����{�����99Zֵ����99�֥���{����J9k��{����{�����  9{���{{��ZZ���֜������{��֌{���k9k��� �� )���  ��{���֜����� ��  c�� ������� ���� )����� Z�  ���� )����� Z�  ����)� !���������� {� ������  )�� 9� )����� �� ����+ �� 1B99RR) �  � )1�9ZB)19JRBJ9!)9BB)1RR!)99!�!  ������ ����  ))c  B)  ����R c9 9���)RB  Bޜ )k  �ތ� ���R B   ���)k c�� c� )c�ތ� ��R k���k� 99B  R) ��� 9� ���  R�  R���)c����� ���ޥk{�J��� 9� �� ��  ��� 9� �� ��  ����� � ���� )��� �� {���)�  )���� )�� R� ��� �� 1B99RR) �  � )1�9ZB)19JRBJ9!)9BB)1RR!)99!�!  ������ ����  ))c  B)  ����R c1 1���)RB  Bޜ )k  �ތ� ���R B   ���)k c�� c� )c�ތ� ��R k���k� 11B  R) ��� 1� ���  R�  R���)c����� ���ޜcs�9��� 1� �� ��  ��� 1� �� ��  ����� � ���� )��� �� s���)�  )���� !�� R� ��� �� 1B99RR) �  � )1�9ZB)19JRBJ9!)9BB)1RR!)99!�!  ������ ����  ))k  J)  ����Z k9 9���)ZJ  J�� ){  ���� ���Z J   ���){ k�� k� )k���� ��Z {���{� 99J  Z) ��� 9� ���  Z�  Z���)k�����  ��������������� 9� )���� ��  ��� 9� )���� ��  �����  ��� ���� )��� �� ������)�  )���� )���� Z� J������� )B9J�9�� (!))!BJBJ)1B1JRJ9)!1B1Bk9)!9B)  �� ����������� ����� ���������������
�  ��B B�����=�޵ �ޥ  B��c  ���c s��B cޥ ���B {�9)�� �  ��B B���  ��)� >��) ��� ����������c  �����R �R � ��� {�ތ  )��s  {��) ���� ���� k�����  ��R  ����� ������� s� �k���֭�!!���������� �� )�������������� Bk��ޜ� )����������� Bk����  �9���� s� B������)� ���������k� 1J����9� J���������	�������)� {����������������� ������������ )B9J�9�� (!))!BJBJ)1B1JRJ9)!1B1Bk9)!9B)  �� � ���  ��B B��;� �ޥ  B��c  ���c k��B cޥ ���B {�9)�޵ ޥ  ��B B���  ��)� 	��) ��� ���c  ���!R �R ޜ ��� {�ތ  )��k  {��) ��� k���  ��R  ����޵ ��� k� � c��֭���� �� !���� 9c�� �� !���� 9c���� � 1�� k�  B�� )� ��� k� )9��� 9� 9�� � s���� )B9J�9�� (!))!BJBJ)1B1JRJ9)!1B1Bk9)!9B)  �� � ���  ��J J��;� ���  J��k  ���k {��J k�� ���J ��9)��� ��  ��J J���  ��)� 	��) ��� ���k  ���!Z �Z �� ��� ����  )��{  ���) ��� {���  ��Z  ������ ��� {� �� ����޽����� �� ������ ���� �� ������ ������  B�� ��� {�  J�� )� c���� {� c���� 9� )������J� )����s� ��)BJJ1!��   �  � 	BRBB9B9B�91)J99)!1919)BB)1�)!  � ����� ��  ��B R��  ���  ���  ���s  ƭ ��� ��9 R��R9� ��  ��{ ��{ ���  ��9 ��s  ��
s R�猜� ���s  ��  ���{  ��s  ���9 ��  ���  ��R 9��  R�� s� � ������� R� J�� s� ��� R� J�� s� ��R� �9��� ��  ����� ����� 9��� J������� ���� ��)BJJ1!��   �  � 	BRBB9B9B�91)J99)!1919)BB)1�)!  � ����� ��� �������� �ޥ  ��B R�޵  ��ޥ  ���  ���k  ƥ ��� ��9 R��R9ޥ �ޥ  ��{ ��{ ��ތ  ��9 ��k  ��
s R�ތ�� ���k  �޵  ���{  ���k  ��� ���9 ��  ���  ��R 9���  R�� k� � ������� ��� ��� R�  9�����k� ����R�  9���k� ��R� 1��� ��  ��� �� ������ 1�ޥ� 9����������ތ�� ����� ���� ��)BJJ1!��   �  � 	BRBB9B9B�91)J99)!1919)BB)1�)!  � ����� ���  ��B Z���  ����  ���  ���{  ֽ ��� ��9 Z��Z9�� ���  ��� ��� ����  ��9 ��{  ��
{ Z����� ���{  ���  ����  ��{  ���9 ��  ����  ��Z 9��  Z�� {� ������ ��� Z� s��� {� {����� Z� s��� {� {����Z� ������ ��  ����� ������ !������� )������������9� B�����@��!!BJJ)� �   ��  � JJ1ZJ9B)ZR1�BJB)�J991)!)�19� ����c���{  �� ���  ����  s�s  c�s  ��� 9 ��R  9 )��c���{  �� B��s B��9  ��c ��� ��s B��{  ��s  ���  ���s ���  ���  �� 9��  �� �{R  R�� s� �筌��� �����s��� �  B�� ��  � ��� �  B�� ��  ���� � ��� �� !��� {� ��� )� �9��)� J��� ���c� � k����!!BJJ)� �   ��  � JJ1ZJ9B)ZR1�BJB)�J991)!)�19� ����c���{  �� ���  ����  s�s  c�s  ��� 9 ��R  9 )��c���{  �� B��s B��9  ��c ��� ��s B��{  ��s  ���  ���s ���  ���  �� 9��  �� �{R  R�� s� �筌��� �����s��� �  1�� �� � ��� �  1�� �� ���� � ��� �� ��� s�  ��� )� �1��)� 9��� ���Z�� k����!!BJJ)� �   ��  � JJ1ZJ9B)ZR1�BJB)�J991)!)�19� ����k����  �� ����  ����  {�{  k�{  ��� 9 ��Z  9 )��k����  ��� J��{ J��9  ��k ��� ��{ J���  ��{  ���  ���{ ���  ���  �� 9��  �� ��Z  Z�� {� !s������� �����{��� !� ��� ��  Z�� ��� !� ��� ��  Z�����  �� ��� ��  k�� �� J��� )� ������)� )����� ������{� Z�����I�� �)19R1�  �� �  � !!BRB)191J19BB9JJc91cB91)B99J9�)  ������RR  �� ����  ���  s�B  99scR��c� 9��� ���RR  ����c ���  9BsR{��� 9��s 9���  ��s  s�  ��� ���  ����  �� ��  ƽ� )9RsB��� ��  � )RR)� 9��� s�  ���� �!��� s�  ���� �!֭� c���)� R�� �  s�� R�  �{�{� J��� 9���έk)� � J���� �)19R1�  �� �  � !!BRB)191J19BB9JJc91cB91)B99J9�)  ������RR  �� ����  ���  s�B  99scR��c� 9��� ���RR  ����c ���  9BsR{��� 9��s 9���  ��s  s�  ��� ���  ����  �� ��  Ƶ� )9RsB��� ��  � )RR)� 9��� k�  ���� ���� k�  ���� �֭� Z���)�  J�� �  s�� R� �s�{�  9��� 9���ƭk!� 9���� �)19R1�  �� �  � !!BRB)191J19BB9JJc91cB91)B99J9�)  ������ZZ  �� ����  ���  {�J  99{kZ��k� 9��� ����ZZ  ����k ���  9J{Z���� 9��{ 9���  ��{  {��  ��� ���  ����  �� ��  ��� )9Z{J��� ��  � )ZZ)� 9��� ��  B���� ������ ��  B���� ������� )������)� Z��� !�  ��� Z�  Z������ ����� 9�����ֽ��9  ������O ��!1BB1�� �� �  �  �  � &!1BZZB)9RR9BcJ)c�Z9RZ99JkcJ19B9!)9� ��  ��R�� )��9 ���  ���  ��  ��������s  ���ƄR  s�R�� )������s  ��� ���� 9��J c���  ��s  ���  ���  ���  ��
�  �罽�  ��  ��9 )�� ����� ��� !�  Z����  � ��� !�  Z����  ���9� ����� ��� �� ����9� �9��J�  1���  R���ε9  � c�� ��!1BB1�� �� �  �  �  � &!1BZZB)9RR9BcJ)c�Z9RZ99JkcJ19B9!)9� ��  ��R�� )��9 ���  ���  ��  ��������s  ����{R  s�R�� )������s  ��� ���� 9��B c���  ��s  ���  ���  ���  ��
�  �罽�  ��  ��9 )�� ����� ��� !�  Z��{� � ��� !�  Z��{� ���9� ����� ��� ��  ����9� �1��B�  )���  R���ƭ1  � Z�� ��!1BB1�� �� �  �  �  � &!1BZZB)9RR9BcJ)c�Z9RZ99JkcJ19B9!)9� ��  ��Z��� )��9 ���  ���  ���  ���������{  ���֌Z  {�Z��� )�������{  ��� ���� 9��J k���  ��{  ���  ���  ���  ��
�  ������  ��  ��9 )�� ����� ��� J�  �����  s�� ��� J�  �����  s����9� !������� Z������ !����9� !������J� ����  Z�����  ������' ��)B9)�  �� � ��  � )1)B�R9BZJ1)JJ1RskBB�9RcR9!�B1!  � � �� ��
� R��  �R���) ���  ���  s�  ������9���J9�J���� �� R��  �Rc)���  ��� ��� ��R s�� ���c ���  ���  ���  ���  ��c��  ����  ��J 9��c9����  ��� �  Z�sc�  � {�� �  Z�sc�  �{�� )��)� ��� {�  �s 9� �!��s� R��� �� ���9 � {�� ��)B9)�  �� � ��  � )1)B�R9BZJ1)JJ1RskBB�9RcR9!�B1!  � � �� ��
� R��  �R���) ���  ���  s�  ������9���J9�J���� �� R��  �Rc)��{  ��� ��� ��R s�� ���c ���  ���  ���  ���  ��c{�  ����  ��J 9��c9����  ��� �  Z�sc� � s�� �  Z�sc� �s�� !��)� ��� s�  �s 9� ���s� R��� ��  ���9  � s�� ��)B9)�  �� � ��  � )1)B�R9BZJ1)JJ1RskBB�9RcR9!�B1!  � � �� ��
� Z���  �Z���) ���  ���  {��  ������9���J9�J���� �� Z���  �Zk)���  ��� ��� ��Z {�� ���k ���  ���  ���  ���  ��k���  ����  ��J 9��k9���1�  ��� 1�  k�{k�  J�� ��� 1�  k�{k�  J������ s����)� s���� ��  )�{ 9� ������{� Z��� �� s���9 !�� ���b��)!�      ��    �  � !199JsRZZJkkZ))RcRRccB!1B19ZR9)�1J)  ����  ��	9 9Μ)  � ���  ���  s��  ��s c���99�� s��Js�R��� �9 9Μ)  � �cRsscR���9 ����s���J  ����  s��� J��� ���s  ���  ���s  )���) ��)��  ���  ������ ��� ��  ��� ��  �!��� ��  �!��s� 99� {�� � �{���  )�� �� 1����c  cR�����)!�      ��    �  � !199JsRZZJkkZ))RcRRccB!1B19ZR9)�1J)  ����  ��	9 9Μ)  � ���  ���  s��  ��s c���99�� s��Js�R��� �9 9Ɯ)  � �cRsscR���9 ����s���J  ����  s��� J��� ���s  ���  ���s  )���) ��)��  ���  ������ ��� ��  ��� �� ���� �� ���s� 99� s�� � �s���  )�� �� )����c  cJ�����)!�      ��    �  � !199JsRZZJkkZ))RcRRccB!1B19ZR9)�1J)  ����  �� ��� ��� ���	9 9ޭ)  � ���  ����  {���  ��{ k���99�� {��J{�Z���� �9 9ޭ)  � �kZ{{kZ���9 ����{���J  ����  {��� J��� ���{  ����  ���{  )��� ���) ��)���  ����  ������� �����)� ����� ����!�  B�� ������!�  B�����{� 99� !�������c� �������  )������ ������k  kε������� ��� ��� ���� ����� 
     � ��  � 9BJJZkJk��kJ99ccZ{�cRBJBJ9JJ!1cB)  �� �� ��� ����c� 9��  s�� �� ��� �� 9��c99)���JR��9)��J����c� 9��  s�)� 
����J99R������9�  s�� J� ����	�9)9���R� ��� Rc�  s�� ���� ��J  ��R  9��999 ��� ��� �������� 1����������c�  � �����c�  �����  �  � {���!� �9���� )���� ��� �  R��	�  )ޥ��� ��� ��� ����� 
     � ��  � 9BJJZkJk��kJ99ccZ{�cRBJBJ9JJ!1cB)  �� �� ��� c� 9��  s�� �� ��� �� 9��c99)���JR��9)��J����c� 9��  s�)� 
����J99R���9�  s�� J�  ���	�9)9���R� ��� Rc�  s��� ��J  ��R  9��999 �����  1�� Z� � ��� Z� �����  � � s���� �1����  )�� �  J��	�  )ޥ��� ����� 
     � ��  � 9BJJZkJk��kJ99ccZ{�cRBJBJ9JJ!1cB)  �� �� ������������ ��������������	��������������k� 9��  {�� ��� �� ��� �� 9��k99)���JZ��9)���J�����k� 9��  {�)� ����J99Z��������9� {���J� �������9)9���Z� ��� Zk� {���!���� ��J  ��Z  9���999 ������������ ��� s� J������������ΌJ!�  B����������ΌJ!�  B�������  �  B���������c9� ��� ��� � 9������������� �����  )�絵�� ������������������������  �  ��   BJJRBBcRRkckk)BJZB9�kZs�J�R!!)sJ ���� ������������������έ��������ބ����������������9��ssJ) �������έ��� ��� ��������޽�����޽�����	�s�������ν���ބ��� ��  �  ��� J�R1� � s�� J�R1� � s�� �� ���sR�  � ��� ��)J� �!���΄J� �c���ޭ����  R��J9������  �  ��   BJJRBBcRRkckk)BJZB9�kZs�J�R!!)sJ ���� ���筜��������������έ����������ބ�������筜�������9��ssJ) ��������������έ��� ��� �����罄���޽����� ���޽�����	�s���眔����ν���ބ��� ��� ��  ��� B�� R)� � k�� B�� R)� � k�� �� � ��sR� � ��� ��� !J�   �����΄J� Z���ޭ����  R��B1�������  �  ��   BJJRBBcRRkckk)BJZB9�kZs�J�R!!)sJ ���� ������ ��� ��������������������������������޽���������������������������������9��{{J) �����������������޽�������������Ό��������������ν���� ����{�������������������� �������� ��� �� !Z9� )������� ���Ƶ�����R� 9c������� ���Ƶ�����R� 9c��������� )Z{��������{sc99) {�� ��� ��������� J�������ތ��sJ�) 9�Z����������Ƶ�����  Z��Ƶ�������� ����������  ��  � �  �  �� +  Rk{cJZ{{ckZZR9Z{c9Bc{kcBcR9cRJ!1)11 �� � ���)���)��Z  ��� ����9� � c���9� � c�� k�1JJ� 9�� {� �!��� 1� 9�� ��!��J9���ss������  ��  � �  �  �� +  Rk{cJZ{{ckZZR9Z{c9Bc{kcBcR9cRJ!1)11 �� � ���)���)�� R� �� ����1�� � Z���1�� � Z��k  �!BB� 1�� {�� ���� !� 1�� ����J9���kk������  ��  � �  �  �� +  Rk{cJZ{{ckZZR9Z{c9Bc{kcBcR9cRJ!1)11 �� � ���)���)��Z  J������Z9)� )��������{)Js��� �������{)Js��� ������������� ��� ����B9Z������� ��� ��� ������J9���ֵ����-�����  �� �� ( !Bkc{ZZJR{k��B)ZkkZBcscJZJZJJZkBBZ! ��� ���� ����Z���Z  s�� Z���s9� c���s9� c�� �� 1� c��ތZ!�!��� 1� J�� !� ����19��������  �� �� ( !Bkc{ZZJR{k��B)ZkkZBcscJZJZJJZkBBZ! ��� ���� ����Z���Z  s�� J���k1� Z���k1� Z�� �� !� Z��ބR���� !� B�� � ����!1��������  �� �� ( !Bkc{ZZJR{k��B)ZkkZBcscJZJZJJZkBBZ! ��� ���� ����Z���Z  ���cJ99� B��������� �������� ��� �� ��� �����ν������ ��� ��� ��� ���ｵ���������� �� �� %9J��csR9Zc�cRsZ)ssRRkssRZ{s{c��R9JB��� 
����s��Js9s�� Z�  ��9�Z�� J�� ��� c�Z��� ��� c�Z��� ��J������������ZZJ1��� ����1�1s���֌���s�J Z�� ���޵������!c������ �� �� %9J��csR9Zc�cRsZ)ssRRkssRZ{s{c��R9JB��� 
����s��Js9s�� Z�  ��1{R�� B�� ��� Z�R��� ��� Z�R��� ��B{����������{RRB!��� ����!�!k���΄���k�B R�{ ���޵������Z������ �� �� %9J��csR9Zc�cRsZ)ssRRkssRZ{s{c��R9JB��� 
����{��J{9{�� Z�  �������� �� s���������������� ��� ������� �����������������Ƶ������޽��������������� ��� ����������ｵ����� ������ �&  )kk��{cJZ�cs�kJBRc{kcZRcJk{Zss{s911����  ���� �� cs�������ZJ!�� J���Js�����Js����ֵ����cZ�����ZJ��� ������ �&  )kk��{cJZ�cs�kJBRc{kcZRcJk{Zss{s911����  ���� �� Zk������{RB�� B���Bk�����Bk����ε����ZR{����RB��� ������ �&  )kk��{cJZ�cs�kJBRc{kcZRcJk{Zss{s911����  ����� �� ε����������ƽ��)  �������������������������������������v��!�� �� � �#!)s�cskk�csk{{sR9{skkc{kZBckJk�sR1��  ���� ��� s� ���	ƌs  c�����!�� �� � �#!)s�cskk�csk{{sR9{skkc{kZBckJk�sR1��  ���� ��� s� ���ƌk� Z�����!�� �� � �#!)s�cskk�csk{{sR9{skkc{kZBckJk�sR1��  ���� ���� {� �����	���9  {��΁����!!��)� �� � � � �% !99k{Zsks�cckkskJB{sccZ{sR9ccBk��9��� !!!��� s�  ��J � ����!!��)� �� � � � �% !99k{Zsks�cckkskJB{sccZ{sR9ccBk��9��� !!!��� s�  ��J  � ����!!��)� �� � � � �% !99k{Zsks�cckkskJB{sccZ{sR9ccBk��9��� !!!���� {� )���J s�� �����!!�!�!��� ��  1Bc��{k���s{��skJRk��kk{c�{ZZ{{s9�  ������ ��� ������������� �������� ��� �������� ��� ������������������������ ��� ��� ��� ������� ������������������������ ��� ��������������� ��� ��  �� ��� ���Z 9J!��������� ��� ��� ���������������������������������������������������������� ��� ��� ��� ��� ��� ���������� ��� ���!!�!�!��� ��   1Bc��{k���s{��skJRk��kk{c�{ZZ{{s9�  ����� �������� ��� ��� ��� ��� ��� ��� ��� �� ��Z 9B��� ��� ��� ��� ��� ���!!�!�!��� ��  1Bc��{k���s{��skJRk��kk{c�{ZZ{{s9�  ����� ��  ���Z 9Ƶ��������!!�!!!��� � � � �� �  1ccs�����{{s�����J{��{sk���Zs{R�!�������� �� ���k  JZ�����!!�!!!��� � � � �� �  1ccs�����{{s�����J{��{sk���Zs{R�!�������� ��  ���k  JR�����!!�!!!��� � � � �� �  1ccs�����{{s�����J{��{sk���Zs{R�!�������� �� J���k  Jε�������!!!� ����� � ��#!1{{k������ks����{Z��kk��kc{{Zs�Z)�� �������� Z�  B��֔{Z� �Z������!!!� ����� � ��#!1�{k������ks����{Z��kk��kc{{Zs�Z)�� �������� Z�  9��֔{Z� �R������!!!� ����� � ��#!1�{k������ks����{Z��kk��kc{{Zs�Z)�� �������� Z� ���ޜ{Z� �ε������!!�
���� ��- !9B{������{s{��s��ZZ���s���{�{{��9) ����������� ����)9�{k� cJ�����!!�
���� ��- !9B{������{s{��s��ZZ���s���{�{{��9) ����������� ����)9�{k� ZB�����!!�
���� ��- !9B{������{s{��s��ZZ���s���{�{{��9) ����������� ����)9�{k� ƵƁ�����!!!!������� ��"  1Bc���{������������sk�������kk�{J!!��� ��!�����9�{� �sZ�����!!!!������� ��#   1Bc���{������������sk�������kk�{J!!��� ��!���� �� 9�{� �kR�����!!!!������� ��"  1Bc���{������������sk�������kk�{J!!��� ��!����֌sZ�sk�{� �Ƶ�΁����!!)!!�!!������  �))�1  1ccs��ƥ�������s{���k��������Zs{R!)���!����� k�!��{kRkk� �����!!)!!�!!������  �)!�1  1ccs��ƥ�������s{���k��������Zs{R!)���!����� c���{kJcc� �����!!)!!�!!������  �))�1  1ccs��ƥ�������s{���k������{�Zs{R!)���!����� �������{s���� ށ�����!!!��!���� ��  �!!�!1{{k�����ƭ����c���s��s����Zs�Z)�!� � �!!�������!�1����!9J��{!�!�����!!!��!���� ��  �!!�$!1�{k�����ƭ����c���s��s��{��Zs�Z)�!� � �!!��������!����1B�{s������!!!��!���� ��  �!!�!1�{k�����ƭ����c���s��s����Zs�Z)�!� � �!!�������������������ֽ�� ������! )� !� �� ������ �� !9B{�������������{��{k��{����{��9)� ��!�!!���������
������������� ��� ������������ ������������������������������������������������� ��������������������������� ������������������������ ����������������� ��������޽��������!����� ��� ��� ��� ���	�������������������� ��������������� ������� ����������������������������������������� ��������������� �������������������! )� !� �� ������ �� !9B{�������������{��{k�������{��9)� ��!�!!����޽��������������! )� !� �� ������ �� !9B{�������������{��{k��{����{��9)� ��!�!!���������������������))�! )�!!!!!!������� -!  1Bc��������������{{��Z������sk�{J!!��!!)!��!!��������� ΁���))�! )�!!!!!!������� -!   1Bc��������������{{��Z������sk�{J!!��!!)!��!!��������� ΁���))�! )�!!!!!!������� -!  1Bc��������������{{��Z������sk�{J!!��!!)!��!!��������� ������!!!� �� � � ��� !!��*  1ccs�����������{{s���c�������Zs{R!)�!�!�1)!���!���������!!!� � �� � ��� !!��*  1ccs�����������{{s���c�������Zs{R!)�!�!�1)!��� !���������!!!� ��� � ��� !!��*  1ccs�����������{{s���c�������Zs{R!)�!�!�1))��� !��������� )�!!�������!�  ��$!1{{k�������s���s�����{�k��{�Zs�Z)�!!�� !��!�1��������)!!!!!������!�  ��$!1�{k�������s���s�����{�k��{�Zs�Z)�!�
!!�!��!�)�������� )�!!�������!�  ��$!1�{k�������s���s�����{�k��{�Zs�Z)�!!!� !��!�)���������1!!!!��!������  �� !9B{���k������������s���{���{��9)!!� � �!!����������1!!!!��!������  �� !9B{���k������������s���{���{��9)!!� �!!����������1!!!!��!������  �� � !9B{���k������������s���{���{
��9)�!� �!!������������! )� !� �� ������ �$ 1Bc�����s�������������{��{k��k�{J!!!)� ��!�!!���������! )� !� �� ������ �$ 1Bc�����{�������������{��{k��k�{J!!!)� ��!�!!���������! )� !� �� ������ �$ 1Bc�����{�������������{��{k��k�{J!!!!)� ��!�!!���������))�! )�!!!!!!������� 7 1ccs�����{������������{{��Z��ss{R!))1)��!!)!��!!��������))�! )�!!!!!!������� 7 1ccs�����{������������{{��Z��ss{R!))1)��!!)!��!!��������))�! )�!!!!!!������� 7 1ccs�����{������������{{��Z��ss{R!))1)��!!)!��!!����������!!!� �� � � ��� !!�!1{{k���������������{{s���c����Z)�! �)!!�!�1)!�!!!!!R���������!!!� � �� � ��� !!�!1�{k���������������{{s���c����Z)�! �)!!�!�1)!�	!!!!!� R���������!!!� ��� � ��� !!�!1�{k���������������{{s���c����Z)�! �)!!�!�1))!!!!� R��������  �  hh����0�G � ������  ��  G  0�         
����0�G       
 "�� @   � �    � � 
         ���  
 G  �   "���   ,  	Helvetica    
 .      0�E0�E +P
designer =  )Q+"Cowboy - jmcald@destiny.esd105.wednet.edu"  ( \ �information ="Scripting by Hybrid.  Teams must be red, blue or yellow.  Up to 3 people on one team. First team to reach 100 seconds in the top center block wins.  Goodies will not regenerate when a hector is on top of the block." *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
    0�E "�6�]   ������ 	         1� �$e #   � �              	�������� 8 � �      "�7�]   33ff�� a��F � < #   � �    wwwwww    h � < � � 
                 
� �o      "�R�]     (� �object Ramp +$y = 0 *thickness = .25  *
deltaY = 3 ( �end � � � � � � � �  
    0�E "��w   ������ 	         1� ��e #   � �              	�������� 8 � �      "��   33ff�� a�>J < #   � �    wwwwww    hJ < � � 
                 
���      #   +1�object Ramp +$y = 0 *thickness = .25  *
deltaY = 3 (�end � � � � � � � �  
    0�E "���}   � � 
          
'. "����   (#wa=2.5  � � wwwwww  
    0�E    A"��� "#%   � �              H � � � �      "����   � � 
          
y� "����   (ywa = 6.8  � � ������  
    0�E 	          8 8 A�� "A   � �              	�������� H � � 
          
53��      "����   (?3object Field  *   deltaY = 0  *   deltaX = 0  *   deltaZ = 0  *   start = @fieldStart *   exit = @goodieRegenOn *end  � � � � 33ff��  
    0�E  $ $ A���� " _    � �              H � �      "���Q   � � "����   � � 
          
=#o "����   (=wa =6.75  � �   ����  
    0�E    A"#�� "D   � �              H � � 
          
i��      "����   (sobject Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (�end � � � � � �  
    0�E "���Q   � � 
          
r#� "����   (r	wa = 6.75 � � ��      
    0�E I "   � �              H � � 
          
2��s      "���Q   (<�object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (��end � � � � � �  
    0�E "����   � � 
          
2#g "����   (2	wa = 6.75 � � ����33  
    0�E I "Y   � �              H � � 
          
5J��      "����   + object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (�Jend � � � � � � � �  
    0�E "���     ���� anq�� � Z #   � �              h � Z � � 
          
�yRQ      "���    	 (�yunique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (�yend *object Goody +$shape = bspGrenade  *y = 2  *yon = 35 *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (Oyend � � � � � �  
    0�E "����   � � 
          
 Ft "����     (
Fwa = 4.5  � � wwwwww  
    0�E    A�� "
A   � �              H � � � �      "����   ������ 	         aEGjl � Z #   � �              	�������� h � Z � � 
          
]L�       "����   ����   +]object Goody  *   shape = bspStandardPill *   y = 9 *   speed = 2 *   missiles = 8  *  grenades = 12  *   boosters = 1  *   yon = 35  *   start = @goodieRegenOn  *end  � � � � � �         
    0�E "����   #   � �       `�K� � Z � � 
          
]:��      "����    	 (e:object Teleporter *
group = -1 *y = -30  *activeRange = 35 * 5 *deadRange = 28 * 5 *destGroup = @recenter  *	shape = 0  *end  *object Teleporter  *y = 30 *	shape = 0  *group = @recenter  *end  � � � � � �  
    0�E "���     ���� an!�F � Z #   � �              h � Z � � 
          
�)R      "���   (�)unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (�)end *object Goody +$shape = bspGrenade  *y = 2  *yon = 35 *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (O)end � � � � � �  
    0�E "����     ���� aqC� � Z #   � �              h � Z � � 
          
<yQ      "����   (Dyunique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (�yend *object Goody +$shape = bspGrenade  *y = 2  *yon = 35 *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (�yend � � � � � �  
    0�E "���   ������ a  4?   Z #   � �              h   Z � � 
          
 . a�      "���     ( %.object Base *   in = @start *   out[0] = @fieldStart  *   out[1] = @goodieRegenOn *end  � � � � � �  
    0�E "��   ��     a&^L&�� � Z #   � �              h � Z � � 
          
&�V0�*      "��    
 (&�Vobject Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  ('_V   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )5blue )Close  ('�V   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  ((CVend *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )9Blue ((�V   n = kTeamScore1  *
   out = @ ))blue )Wins ((�Vend *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )Wins ()�V   showEveryone = true  *   text = "  )%Blue ) team wins!" ()�Vend *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true */   text = "Yellow team has reached the halfway   *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *.   text = "Yellow team has only 10 seconds to  *go!" *end  � � � � � �  
    0�E "��   ��     a&gQ&�� � Z #   � �              h � Z � � 
          
&�b-7      " ��   (&�bobject Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ))blue )At40 ('�b   restart = false  *end  *object Counter *   in = @add1  )9Blue ('�b	   n = 50 *
   out = @ ))blue )At50 ('�b   restart = false  *end  *object Counter *   in = @add1  )9Blue (()b	   n = 60 *
   out = @ ))blue )At60 ((Ab   restart = false  *end  *object Counter *   in = @add1  )9Blue ((qb	   n = 70 *
   out = @ ))blue )At70 ((�b   restart = false  *end  *object Counter *   in = @add1  )9Blue ((�b	   n = 80 *
   out = @ ))blue )At80 ((�b   restart = false  *end  *object Counter *   in = @add1  )9Blue ()b	   n = 90 *
   out = @ ))blue )At90 ()b   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At10 ()ab   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 10 seconds!" ()�bend *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 20 seconds!" ()�bend *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 30 seconds!" (*Ebend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At40 (*�b   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 40 seconds!" (*�bend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At50 (*�b   showEveryone = true  *   text = "  )%Blue ) team has reached the halfway  (*�bmark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At60 (+Ab   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 60 seconds!" (+ebend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At70 (+�b   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 70 seconds!" (+�bend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At80 (,b   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 80 seconds!" (,%bend *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At90 (,ab   showEveryone = true  *   text = "  )%Blue )! team has only 10 seconds to go!"  (,ybend � � � � � �  
    0�E "��   ��     a&` P&� � � Z #   � �              h � Z � � 
          
&� Z,�%      "��   (&� Zobject Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *,   text = "Red team has reached the halfway  *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
    0�E "���   ��     a&X+&�v � Z #   � �              h � Z � � 
          
&�4*��      "��   (&�4object Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
    0�E "���   ��     a&Q&�a � Z #   � �              h � Z � � 
          
&�!)0      "��   (&�!object Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
    0�E "��&   ��ffff    A-�-2 #   � �              H � � 
         ��ffff 1,o�-B         
,o�-B      #     (,y�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    0�E "���   ����33 A-o-6� #   � �              H � � 
         ����33 1,o5-�         
,o5-�      #   (,y5object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    0�E "��r     ���� A-)-5R #   � �              H � � 
           ���� 1,o�-�         
,o�-�      #   (,y�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    0�E "���     ��   a �*|�  < #   � �              h  < � � 
          
�H      "���   ((�object Incarnator *y = 30 *end  � � � � � �  
    0�E "���     ��   a6 ��� � < #   � �              h � < � � 
          
v8��      "����   (�8object Incarnator *y = 30 *end  � � � � � �  
    0�E "�~�     ��   a � �{� < #   � �              h < � � 
          
#^�      "����   , 	 Monaco    (*#object Incarnator *y = 30 *end  � � � � � �  
    0�E "����     ��   a4)�� i < #   � �              h i < � � 
          
���      "��
     (��object Incarnator *y = 30 *end  � � � � � �  
    0�E "��   ����33 a   1 y � #   � �    ff����    h � � � 
                 
 	 ~        "���   (  ~adjust SkyColor end � � � � 33����  
    0�E i Z � "  p   � �       h Z � � � 
                 
  ~ )       "���   *adjust GroundColor end � � � �  
    0�E "�9��     ��   a�N9� Z #   � �              h Z � � 
          
��nI      "�]�   , 	 Geneva    (��object Incarnator *y = 30 *stop = @start  *end  * object Incarnator  *y = 33 *stop = @start  *end  � � � � � �  
    0�E "�u��   � � 
          
�� "���     (�wa=2.5  � � wwwwww  
    0�E    A �$�� " �M   � �              H � � � �      "�{��   � � 
          
�� "���   (�wa=2.5  � � wwwwww  
    0�E A�$�� "M   � �              H � � � �      "���w   � � 
          
 '( "����   (# wa=2.5  � � wwwwww  
    0�E A" ��� "#   � �              H � � � �      "���   ������ 	         atH�m � Z #   � �              	�������� h � Z � � 
          
�eU=      "���    	 (�eunique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (�eend *object Goody +$shape = bspMissile  *y = 5  *yon = 45 *	speed = 2  *missiles = 8 *start = mShow  *out = mTaken (Reend � � � � � �  
    0�E "����   ������ 	         aFk? � Z #   � �              	�������� h � Z � � 
          
a7'      "����   (i7unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (�7end *object Goody +$shape = bspMissile  *y = 5  *	speed = 2  *yon = 45 *missiles = 8 *start = mShow  *out = mTaken ($7end � � � � � �  
    0�E "����   ������ 	         aH=m � Z #   � �              	�������� h � Z � � 
          
3e�=      "����   (;eunique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (�eend *object Goody +$shape = bspMissile  *y = 5  *yon = 45 *	speed = 2  *missiles = 8 *start = mShow  *out = mTaken (�eend � � � � � �  
    0�E "����   ������ 	         aFvk� � Z #   � �              	�������� h � Z � � 
          
a�'k      "���   (i�unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (��end *object Goody +$shape = bspMissile  *y = 5  *yon = 45 *	speed = 2  *missiles = 8 *start = mShow  *out = mTaken ($�end � � � � � �  
    0�E "����     ���� a!CF � Z #   � �              h � Z � � 
          
<)      "����   (D)unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 150 (�)end *object Goody +$shape = bspGrenade  *y = 2  *yon = 35 *	speed = 2  *grenades = 12  *start = mShow  *out = mTaken (�)end � � � � � �  
    0�E "����   � � 
          
�� "���   ����     (�floorTemplate = bspFloorFrame *	wa = 6.75  *wallHeight = 0 *wallYon = 40 � �         
    0�E 1"$�� "We   � �       8 � � � �      "�u�8   ������ 	         1 ��c& #   � �              	�������� 8 � �      "�~�D   33ff�� a ��B� < < #   � �    wwwwww    h < < � � 
                 
�SS      "��O   (�object Ramp +$y = 0 *thickness = .25  *
deltaY = 3 (N�end � � � � � � � �  
    0�E "�u��   ������ 	         1 ��c� #   � �              	�������� 8 � �      "�~��   33ff�� a ��A � < #   � �    wwwwww    h � < � � 
                 
~U      "���   (~object Ramp +$y = 0 *thickness = .25  *
deltaY = 3 (Q~end � � � � � � � �  
    0�E "���8   ������ 	         1N��& #   � �              	�������� 8 � �      "���D   33ff�� aA��� < < #   � �    wwwwww    h < < � � 
                 
g��S      "���O   (q�object Ramp +$y = 0 *thickness = .25  *
deltaY = 3 (��end � � � � � � � �  
    0�E "����   ������ 	         1N��� #   � �              	�������� 8 � �      "����   33ff�� aA�� � < #   � �    wwwwww    h � < � � 
                 
j~�      "����   (t~object Ramp +$y = 0 *thickness = .25  *
deltaY = 3 (�~end � � � � � � � �  
    0�E "���   ������ 	         1�P�� #   � �              	�������� 8 � �      "���   33ff�� a�b�J < #   � �    wwwwww    hJ < � � 
                 
�i��      #   (�iobject Ramp +$y = 0 *thickness = .25  *
deltaY = 3 (�iend � � � � � � � �  
    0�E "�6��   ������ 	         1�P$� #   � �              	�������� 8 � �      "�7��   33ff�� a�c�� � < #   � �    wwwwww    h � < � � 
                 
�8�      "�R��   (�8object Ramp +$y = 0 *thickness = .25  *
deltaY = 3 (8end � � � � � � � �  
    0�E "�*�   � � "����   ����33 a Xd < #   � �              h < � � 
          
 ~f&      #   (  ~object Walker +$lives = kDefaultLives *team = 2 *y = 30 (X ~end � � � � � �  
    0�E "�*�i   ����33 a �� �� < #   � �              h < � � 
          
 � � ��      #   ( � �object Walker +$lives = kDefaultLives *team = 2 *y = 30 ( � �end � � � � � �  
    0�E "��   ����33 a� �� � < #   � �              h < � � 
          
� � �      #   (� object Walker +$lives = kDefaultLives *team = 2 *y = 30 (� end � � � � � � � �  
    0�E "���   � � #   ����33 a� �, � � < #   � �              h � < � � 
          
! g �      "���   (+ object Walker +$lives = kDefaultLives *team = 3 *y = 30 (c end � � � � � �  
    0�E "���   ����33 aX �d � < #   � �              h � < � � 
          
� ��0      "�	�   +m0object Walker +$lives = kDefaultLives *team = 3 *y = 30 (� �end � � � � � �  
    0�E "�h�q   ����33 a��� � < #   � �              h � < � � 
          
� �@�      "�z�q   +i9object Walker +$lives = kDefaultLives *team = 3 *y = 30 (< �end � � � � � � � �  
    0�E "����   � � "���^   ����33 a��)� i < #   � �              h i < � � 
          
�Q�      "���s   (�object Walker +$lives = kDefaultLives *team = 6 *y = 30 (M�end � � � � � �  
    0�E "�b��   ����33 a�� % i < #   � �              h i < � � 
          
�(�      "�b��   (�object Walker +$lives = kDefaultLives *team = 6 *y = 30 ($end � � � � � �  
    0�E "����   ����33 aVJ�� i < #   � �              h i < � � 
          
t��*      "���   (~�object Walker +$lives = kDefaultLives *team = 6 *y = 30 (��end � � � � � � �   �Hull Res IDDWRDMax MissilesDWRDMax GrenadesDWRDMax boostersDWRDMassDLNG
Max EnergyDLNGEnergy ChargeDLNGMax ShieldsDLNGShield ChargeDLNGMin ShotDLNGMax ShotDLNGShot ChargeDLNGRiding HeightDLNGAccelerationDLNG
Jump PowerDLNG  �GZRG 	kuwtKuwait LStay in the center square for 100 seconds... Teams are yellow, red, or blue. baghdad2.pict          sand
SandStorm! JStay in the center square for 100 seconds. Teams are yellow, red, or blue. 	sand.pict          kotrKing of the Ring SStay in the center lower square 100 seconds to win. Teams are yellow, red, or blue.kingring.pict          shw2Koth ShowdownnStay in the square anywhere between the floor and the ceiling for 100 seconds. Teams are yellow, red, or blue. showdown2.pict           nothNothingVNot much here, just a square, a grenade and a missle.. Teams are yellow, red, or blue. nothing.pict           32pl32nd Parallel�Teams must be red and yellow.  Up to 3 people on one team. Protect your side from your opponent - first to reach 100 seconds alone on their side wins. 	line.pict          brnaBurned Again!LStay in the lower square 100 seconds to win. Teams are yellow, red, or blue. burnedagain.pict           mirsMirage QStay on the center platform 100 seconds to win... Teams are yellow, red, or blue.mirage.pict          rodeRodeoiHow long can you stay on the center block? It takes 100 seconds to win... Teams are yellow, red, or blue.
rodeo.pict             ]$]$���� � � ������  ��  �            
���� �       
 "�� @   � �    � � 
          
 q u�   "���   , 	 Geneva     
 .       }� }� +{designer ="The Head < , 	 Monaco     	 )jtra@idcomm.com    
 )T>" ( � �information ="Another fine level brought to you by the geezers...  Scripting by Hybrid. A fast rendering, fast playing level. Teams must be red, blue or yellow.  Up to 3 people on one team. First team to reach 100 seconds in the center square wins." *missilePower = 1.8 *missileAcceleration = 0.3  *
startY = 9 *// grenades  *grenDelay = 250  *numGren = 6  *// missiles  *missDelay = 250  *numMiss = 4  *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
     }� "�>�T   ����33 a�l�� � < #   � �              h � < � � 
          
�� |      "�P�T     (��object Walker +$lives = kDefaultLives *team = 3 *
y = startY (�end � � � � � �  
     }� "����   ����33 a�M� � < #   � �              h � < � � 
          
B!��      "����   +M2object Walker +$lives = kDefaultLives *team = 3 *
y = startY (�!end � � � � � �  
     }� "�!�   ����33 a��_ � < #   � �              h � < � � 
          
��+      "�3�   +b1object Walker +$lives = kDefaultLives *team = 3 *
y = startY (��end � � � � � �  
     }� "���^   � � "����   ��     aW-� , #   � �    ����      h , � � 
                 
�wt      "���   ,  	Helvetica    	 (�(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (l�end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (��end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (�end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (t�end � � � � � �  
     }� "�$��   ��     a��.J, #   � �    ����      hJ, � � 
                 
�1      "�.��   (�1(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (1end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (f1end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (�1end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (1end � � � � � �  
     }� "���   ��     a���� Z, #   � �    ����      h Z, � � 
                 
�#/      "�O��   (�#(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay ($#end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (�#end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (�#end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (,#end � � � � � �  
     }� "�Y�^   ��     a��� �, #   � �    ����      h �, � � 
                 
��A�      "�a�^   (��(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (6�end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (��end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (��end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (>�end � � � � � �  
     }� "�Q��   ��     a��., #   � �    ����33    h, � � 
                 
�89$      "�Y��   (�8(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (.8end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (�8end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (�8end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (68end � � � � � �  
     }� "���.   ��     alW�� �, #   � �    ����33    h �, � � 
                 
1��z      #   (9�(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (��end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (��end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (6�end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (��end � � � � � � � �  
     }� "�g��   ����   1��� #   � �              8 � � 
          
�:�c      "�g��       (�:wa = 0  � � � � 
          
0�� "����   (!0object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (�0end � � � � � �  
     }� "�h��   � � 
          
�]�� "�h��   (�]wa = 0  � � ��ffff  
     }� 9 "�E   � �              8 � � 
          
'��      "����   ('object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (~'end � � � � � �  
     }� "�k��   � � 
          
�i�� "�k��   (�iwa = 0  � �   ����  
     }� 9 "�E   � �              8 � � 
          
*��      "����   ($*object Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (�*end � � � � � �  
     }� "�N��   � � 
          
�� #   (�floorTemplate = bspFloorFrame *baseHeight = 0 *wallHeight = 0 *wallYon = 40 � �  
     }� 9 "
T   � �       8 � � � �      "��   ��     a   1 | � #   � �    ����      h � � � 
                 
  �  �      "��      	 (  �adjust SkyColor end � � � � � �  
     }� "��   ��ff33 i Z � #   � �    DDDDDD    h Z � � � 
                 
  � *      "��   ( ( �adjust GroundColor end  � � � � � �  
     }� "����     ff   a{E�n, < #   � �       h, < � � 
                 
�O��      "���     (�Oobject Incarnator   *y = startY end � � � � � �  
     }� "����     ff   aI.r � < #   � �       h � < � � 
                 
B%�      #   (Bobject Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   a7m`� < < #   � �       h < < � � 
                 
,jM�      #   (4jobject Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   a(NQw � < #   � �       h � < � � 
                 
.EO�      #   (6Eobject Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   au-� x < #   � �       h x < � � 
                 
p%�      #   (pobject Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   asj��   < #   � �       h   < � � 
                 
xn��      "����   (�nobject Incarnator *  y = startY *end  � � � � � �  
     }� "���g   #   � �       ` ��:F � Z � � 
          
�gn      "���g     (#�object Sound  *  y = 50 *  sound = snArcticWind *  isAmbient = true *  loopCount = -1 *  volume = 80  *end  � � � �  
     }� " L    � �   @   ����     #b��             #   � �   @   ����     �{Y             #   � �   @   ����      �l�l             #   � �   @   ����    P���             #   � �   @   ����    P����             #   � �   @   ����    P ��� � �             "����   a6Yb� < < #   � �       h < < � � 
          
QJ��      "����     ����   (YJobject Teleporter *  group = @center y = 2*startY *  activeRange = 200  *  deadRange =  55  *  destGroup = @recenter  *  shape = 0  *end  *object Teleporter  *  y = startY * group = @recenter *  shape = bspGroundStar  *  speed = 0  *end  *object Hologram end  � � � � � �         
     }� "	��   ��     a3�~� � Z #   � �              h � Z � � 
          
k� yi      "	��      
 (u�object Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )?blue )Close  (E�   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )?blue )Close  (��   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )?blue )Close  (<�end *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )?Blue (��   n = kTeamScore1  *
   out = @ ).blue )Wins (��end *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )Wins (��   showEveryone = true  *   text = "  ).Blue ) team wins!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true *'   text = "Yellow team has reached the   *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *+   text = "Yellow team has only 10 seconds   *to go!"  *end  � � � � � �  
     }� "	��   ��     a<��� � Z #   � �              h � Z � � 
          
u��v      "	��!   (�object Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ).blue )At40 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (��	   n = 50 *
   out = @ ).blue )At50 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (�	   n = 60 *
   out = @ ).blue )At60 (9�   restart = false  *end  *object Counter *   in = @add1  )?Blue (m�	   n = 70 *
   out = @ ).blue )At70 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (��	   n = 80 *
   out = @ ).blue )At80 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (	�	   n = 90 *
   out = @ ).blue )At90 (#�   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At10 (q�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 10 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  ).Blue ) team has 20 seconds!" ( �end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  ).Blue ) team has 30 seconds!" (h�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At40 (��   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 40 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At50 (�   showEveryone = true  *   text = "  ).Blue ) team has reached the halfway  (+�mark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At60 (y�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 60 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At70 (��   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 70 seconds!" (�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At80 (I�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 80 seconds!" (p�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At90 (��   showEveryone = true  *   text = "  ).Blue ) team has only 10 seconds to   (��go!"  *end  � � � � � �  
     }� "	��   ��     a5 �� � � Z #   � �              h � Z � � 
          
f ��d      "	��   (p �object Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *$   text = "Red team has reached the  *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
     }� "	���   ��     a-jx� � Z #   � �              h � Z � � 
          
bs      "	���   (lsobject Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
     }� "	���   ��     a&Uq� � Z #   � �              h � Z � � 
          
\`�o      "	���   (f`object Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
     }� "��e   ��ffff    A�E #   � �              H � � 
         ��ffff 1D���         
D���      #       (N�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
     }� "���   ����33 A��� #   � �              H � � 
         ����33 1Dt�         
Dt�      #   (Ntobject WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
     }� "���     ���� A�h
� #   � �              H � � 
           ���� 1D1��         
D1��      #   (N1object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
     }� "�L��   ����33 a�"�f _ < #   � �              h _ < � � 
          
�Z      "�L��     (�Zobject Walker +$lives = kDefaultLives *team = 6 *
y = startY (Zend � � � � � �  
     }� "�+�   ����33 a�w�� i < #   � �              h i < � � 
          
���W      "�+�/   (��object Walker +$lives = kDefaultLives *team = 6 *
y = startY (��end � � � � � �  
     }� "����   ����33 a�[$ i < #   � �              h i < � � 
          
=��      "����   (Iobject Walker +$lives = kDefaultLives *team = 6 *
y = startY (�end � � � � � �  
     }� "���k   ����33 aT��.T < #   � �              hT < � � 
          
\���      "���k   (h�object Walker +$lives = kDefaultLives *team = 2 *
y = startY (��end � � � � � �  
     }� "����   ����33 aG?��J < #   � �              hJ < � � 
          
O[�      "����   ([[object Walker +$lives = kDefaultLives *team = 2 *
y = startY (�[end � � � � � �  
     }� "���A   ����33 aT��@ < #   � �              h@ < � � 
          
\��s      "���K   (h�object Walker +$lives = kDefaultLives *team = 2 *
y = startY (��end � � � � �  ll����$| � ������  ��    $|         
����$|       
 "�� @   � �    � � 
          
 � K   "�m��   , 	 Geneva      .      $z$z +K�lightHull = 150 *mediumHull = 150 *heavyHull = 150  � � � � 
          
 < P �� "���   ,  	Helvetica    
 ( E P5designer ="Cowboy <jmcald@destiny.esd105.wednet.edu>" *�information ="Scripting by Hybrid. Teams must be red, blue or yellow.  Up to 3 people on one team. First team to reach 100 seconds in the center square wins." *missilePower = 3.5 *missileAcceleration = 0.3  *grenadePower = 2.5 *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
    $z "����   ffffff a6 f� �   Z #   � �              h   Z � � 
          
b eu �      "����    	 +�
object And  *in = @start  *	out=@time  *end  *object Counter *in[0]=@zero  *
in[1]=@one *
in[2]=@two *in[3]=@three *in[4]=@four  *	out=@time  *end  *object Timer *in=@time *	out=@show  *wait=175 *end  *object Distributor *in=@show *out[0]=@zero *out[1]=@one  *out[2]=@two  *out[3]=@three  *out[4]=@four *end  � � � � � �  
    $z "���3   � � "���3   � � 
          
l�z� #     (v�wa = 0  � � ��      
    $z 1x�T� "v�   � �              8 � � 
          
���      "�-�]   +*Aobject Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (�end � � � � � �  
    $z "���3   � � 
          
kyT "����   (uwa = 0  � � ����33  
    $z 1x�S� "u�   � �              8 � � 
          
���      "�$�g   (��object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (�end � � � � � �  
    $z "���3   � � 
          
mV{� "����   (wVwa = 0  � �   ����  
    $z 1x�T� "w�   � �              8 � � 
          
���      "�)�m   (��object Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (�end � � � � � � � �  
    $z "���3   � � 
          
d�rS "���Y   (n�wallHeight = 0  � � �̙�    
    $z 1w�S� "n   � �              8 � � � �      "�f�     ff�� a��Z � Z #   � �              h � Z � � 
          
���r      "�w�    	 (��object Teleporter *
group = -1 *y = -30  *activeRange = 35 * 5 *deadRange = 28 * 5 *destGroup = @recenter  *	shape = 0  *end  *object Teleporter  *y = 30 *	shape = 0  *group = @recenter  *end  � � � � � �  
    $z "�T�W   a��	H < < #   � �       h < < � � 
          
���      "�m�W     ���� (��object Hologram +$y = 0 *shape = bspGroundStar  (� end  � � � � � �         
    $z "��]   ��     a1�|' � Z #   � �              h � Z � � 
          
i�$w�      "��f    
 (r�object Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (2�   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )5blue )Close  (z�   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )5blue )Close  (�end *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )9Blue (v�   n = kTeamScore1  *
   out = @ ))blue )Wins (��end *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )Wins (Z�   showEveryone = true  *   text = "  )%Blue ) team wins!" (r�end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true */   text = "Yellow team has reached the halfway   *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *.   text = "Yellow team has only 10 seconds to  *go!" *end  � � � � � �  
    $z "��b   ��     a:��, � Z #   � �              h � Z � � 
          
s� ��      "��r   (|�object Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ))blue )At40 (��   restart = false  *end  *object Counter *   in = @add1  )9Blue (��	   n = 50 *
   out = @ ))blue )At50 (��   restart = false  *end  *object Counter *   in = @add1  )9Blue (��	   n = 60 *
   out = @ ))blue )At60 (�   restart = false  *end  *object Counter *   in = @add1  )9Blue (D�	   n = 70 *
   out = @ ))blue )At70 (\�   restart = false  *end  *object Counter *   in = @add1  )9Blue (��	   n = 80 *
   out = @ ))blue )At80 (��   restart = false  *end  *object Counter *   in = @add1  )9Blue (��	   n = 90 *
   out = @ ))blue )At90 (��   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At10 (4�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 10 seconds!" (X�end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 20 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  )%Blue ) team has 30 seconds!" (�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At40 (T�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 40 seconds!" (x�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At50 (��   showEveryone = true  *   text = "  )%Blue ) team has reached the halfway  (��mark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At60 (�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 60 seconds!" (8�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At70 (t�   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 70 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At80 (��   showEveryone = true  *   mask = T6 *   text = "  )%Blue ) team has 80 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  )"blue )At90 ( 4�   showEveryone = true  *   text = "  )%Blue )! team has only 10 seconds to go!"  ( L�end � � � � � �  
    $z "��a   ��     a3 �~+ � Z #   � �              h � Z � � 
          
d � ��      "��j   (m �object Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *,   text = "Red team has reached the halfway  *mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
    $z "��<   ��     a+�v � Z #   � �              h � Z � � 
          
`�}V      "��D   (i�object Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
    $z "��'   ��     a$�o� � Z #   � �              h � Z � � 
          
Z���      "��1   (c�object Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
    $z "���   ��ffff    A �m!� #   � �              H � � 
         ��ffff 1 B6 ��         
 B6 ��      #     ( L6object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    $z "��E   ����33 A ��!	( #   � �              H � � 
         ����33 1 B� �a         
 B� �a      #   ( L�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    $z "��     ���� A ��!� #   � �              H � � 
           ���� 1 B� �         
 B� �      #   ( L�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � �   ����  
    $z a  = - � � " �    � �       h � � � 
                 
  � X      "��6   (  �adjust SkyColor end � �   ffff  
    $z i Z � "  p   � �    ��        h Z � � � 
                 
  � %X      "��6   *adjust GroundColor end � � � �  
    $z "���   � � "���   ����33 a
 �O � � < #   � �              h � < � � 
          
( x �      "���   (2 object Walker +$lives = kDefaultLives *team = 2 *y = 30 (j end � � � � � �  
    $z "�,�   ����33 a� �� � � < #   � �              h � < � � 
          
�  �      "�8�   (� object Walker +$lives = kDefaultLives *team = 2 *y = 30 (� end � � � � � �  
    $z "���   ����33 a* �o � � < #   � �              h � < � � 
          
H � �      "���   +Xobject Walker +$lives = kDefaultLives *team = 2 *y = 30 (� end � � � � � � � �  
    $z "����   � � "����   ����33 a Ze� � < #   � �              h � < � � 
          
^v�      "����   (hvobject Walker +$lives = kDefaultLives *team = 6 *y = 30 (�vend � � � � � �  
    $z "���c   ����33 a �e) � < #   � �              h � < � � 
          
^���      "���f   (h�object Walker +$lives = kDefaultLives *team = 6 *y = 30 (��end � � � � � �  
    $z "����   ����33 a :e� � < #   � �              h � < � � 
          
^V��      "����   (hVobject Walker +$lives = kDefaultLives *team = 6 *y = 30 (�Vend � � � � � � � �  
    $z "����   � � "����   ����33 a c[ ��J < #   � �              hJ < � � 
          
 kw �      "����   ( uwobject Walker +$lives = kDefaultLives *team = 3 *y = 30 ( �wend � � � � � �  
    $z "���d   ����33 a c� �*J < #   � �              hJ < � � 
          
 k� ��      "���g   ( u�object Walker +$lives = kDefaultLives *team = 3 *y = 30 ( ��end � � � � � �  
    $z "����   ����33 a c; ��J < #   � �              hJ < � � 
          
 kW ��      "����   ( uWobject Walker +$lives = kDefaultLives *team = 3 *y = 30 ( �Wend � � � � � �   ""    
    $z    A��O� " C    � �              H   ""   A���?      #   � �              H   ""   A���      #   � �              H   ""   A|���      #   � �              H   ""   AR>��      #   � �              H   ""   A�'}�      #   � �              H   ""   AM&��      #   � �              H   ""   A��}      #   � �              H   ""   AO��      #   � �              H � �      "���   ��ff   ayU�� � Z #   � �              h � Z � � 
          
�]Y�      "���    	 (�]unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 100 (�]end *object Goody +$shape = bspMissile  *y = 5  *	speed = 2  *missiles = 4 *start = mShow  *out = mTaken (U]end � � � � � �  
    $z "���0   ��ff   a�R� � Z #   � �              h � Z � � 
          
<��      "���7   (D�unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 100 (��end *object Goody +$shape = bspMissile  *y = 5  *	speed = 2  *missiles = 4 *start = mShow  *out = mTaken (��end � � � � � �  
    $z "����       �� aTQ� � Z #   � �              h � Z � � 
          
?W��      "����   (GWunique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 100 (�Wend *object Goody +$shape = bspGrenade  *y = 5  *	speed = 2  *grenades = 6 *start = mShow  *out = mTaken (�Wend � � � � � �  
    $z "��2       �� az��� � Z #   � �              h � Z � � 
          
��^      "�"�4   (��unique mShow mTaken end *&object And in = @start out = mShow end *object Timer +$in = mTaken *out = mShow  *
wait = 100 (��end *object Goody +$shape = bspGrenade  *y = 5  *	speed = 2  *grenades = 6 *start = mShow  *out = mTaken (Z�end � � � � � �  
    $z "���<   #   � �    DD        0M��� � �             "����     ff   ap���c 
 #   � �    ������    hc 
 � � 
                 
\3�x      "����     (f3object Ramp +$y = 0 (�3thickness =   *0  *
deltaY = 3 *end  � � � � � �   ""    
    $z AN��q " m    � �              H � �      "����   #   � �    DD        0Mm�� � �             "���a     ff   ap��Kc 
 #   � �    ������    hc 
 � � 
                 
\��&      "���c   (f�object  *Ramp +$y = 0 (��thickness =   *0  *
deltaY = 3 *end  � � � � � �   ""    
    $z A��1 " {    � �              H   ""   A��~n      #   � �              H   ""   A�'-�      #   � �              H � �      "����   #   � �    DD        0{&�� � �             "����     ff   a.�,B U 
 #   � �    ������    h U 
 � � 
                 
I�      "����   (object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (Eend � � � � � � � �  
    $z "���}   #   � �    DD        0+&P� � �             "�3�}     ff   a���9 U 
 #   � �    ������    h U 
 � � 
                 
���      "�3��   (�object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (�end � � � � � � � �  
    $z "�`��   #   � �    DD        0�k~� � �             "�`�-     ff   a�� � 
 #   � �    ������    h � 
 � � 
                 
��8W      "�q�-   (��object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (3�end � � � � � � � �  
    $z "�N�<   #   � �    DD        0��~� � �             "�N�|     ff   a�RE � 
 #   � �    ������    h � 
 � � 
                 
��&�      "�_�|   (��object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (!�end � � � � � � � �  
    $z "���   #   � �    DD        0.�Q � �             "�R�-     ff   a���l	 
 #   � �    ������    h	 
 � � 
                 
�� W      "�Y�-   (��object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (�end � � � � � � � �  
    $z "���   #   � �    DD        0{�� � �             "���-     ff   a?�=l	 
 #   � �    ������    h	 
 � � 
                 
0�vW      "���-   (:�object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (r�end � � � � � �  
    $z " C    � �      ����    0Q��� � �             "���R     ff   a(� � 
 #   � �    ������    h � 
 � � 
                 
b��|      "���R   (l�object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (��end � � � �   ""    
    $z A��x " C    � �              H      #   � �      ����    0v�T � �             "����     ��   a�� � < #   � �              h � < � � 
          
O'y�      "����     (Y'object Incarnator *y = 30 *end  � � � � � �  
    $z "��Y     ��   a E� �?J < #   � �              hJ < � � 
          
 >� h_      "��g   ( H�object Incarnator *y = 30 *end  � � � � � �  
    $z "�;�     ff   a�U�� U 
 #   � �    ������    h U 
 � � 
                 
��D      "�;�      (��object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (��end � � � � � �  
    $z "�m��     ��   a� }M � < #   � �              h � < � � 
          
� �"      "�x�!     ( �object Incarnator *y = 30 *end  � � � �   ""    
    $z AR�� " '    � �              H   ""   A�>x�      #   � �              H � �      "��     ��   a|=�� < < #   � �              h < < � � 
          
���      "��   (��object Incarnator *y = 30 *end  � � � � � �  
    $z "���W     ��   a��? � < #   � �              h � < � � 
          
N�xO      "���W   (X�object Incarnator *y = 30 *end  � � � �  
    $z " '    � �      ����    0��y� � �             "���     ��   a E ��J < #   � �              hJ < � � 
          
 >7 h�      "���   ( H7object Incarnator *y = 30 *end  � � � � � �  
    $z "����     ff   a,!�c 
 #   � �    ������    hc 
 � � 
                 
^�      "����      ("object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (Zend � � � � � �  
    $z "���     ��   a| }� � < #   � �              h � < � � 
          
� ��      "�(�!     (� �object Incarnator *y = 30 *end  � � � �  
    $z " '    � �      ����    0v<T� � �             "�Q��     ff   a�U��	 
 #   � �    ������    h	 
 � � 
                 
��      "�X��      (�object Ramp +$y = 0 *thickness = 0  *
deltaY = 3 (end � � � � � �  
    $z "�m�     ��   a�>M� < < #   � �              h < < � � 
          
��      "�n�     (��object Incarnator *y = 30 *end  � � � � � �  
    $z "����   ������ a)dN� � Z #   � �              h � Z � � 
          
?i�A      "����     ff��     	 (Riobject Goody  +$shape = bspStandardPill *y = 5  *	speed = 2  *grenades = 12  *missiles = 8 *boosters = 1 *start = @two *stop = @three  (�iend � � � � � �         
    $z "���   ������ ae�� � Z #   � �              h � Z � � 
          
�jB      "���     ff�� +�object Goody  +$shape = bspStandardPill *y = 5  *	speed = 2  *grenades = 12  *missiles = 8 *boosters = 1 *start = @three *stop = @four (jend � � � � � �         
    $z "���7   ������ a)�N� � Z #   � �              h � Z � � 
          
?���      "���;     ff�� (R�object Goody  +$shape = bspStandardPill *y = 5  *	speed = 2  *grenades = 12  *missiles = 8 *boosters = 1 *start = @four  *stop = @zero (��end � � � � � �         
    $z "��5   ������ a~��� � Z #   � �              h � Z � � 
          
���      "��9     ff�� (��object Goody  +$shape = bspStandardPill *y = 5  *	speed = 2  *grenades = 12  *missiles = 8 *boosters = 1 *start =@one  *stop = @two  (
�end � � � � � �         
    $z "�f��   ������ a��3 � Z #   � �              h � Z � � 
          
�b�      "�i��     ff�� (�object Goody  +$shape = bspStandardPill *y = 5  *	speed = 2  *grenades = 12  *missiles = 8 *boosters = 1 *start = @zero  *stop = @one  (_end � � � � �   4 �    ~�� 	'� �� �� �� p  �` 
�`  a�  �� 8�  ^�^����� � � ������  ��  �            
���� �       
 "�� @   � �    � � 
          
 o ��   "���   , 	 Geneva     
 .       }� }� +ydesigner ="The Head < , 	 Monaco     	 )jtra@idcomm.com    
 )T>" ( � information = )D�" Scripting by Hybrid. Kuwait at dusk, with non-targeting mega-missiles (scuds).  Teams must be red, blue or yellow.  Up to 3 people on one team. First team to reach 100 seconds in the center square wins."  ( � ambient = 0.4 *1light[0].i = 1.0  light[0].a =  0  light[0].b = 0  *1light[1].i = 0.0  light[1].a =  0  light[1].b = 0  *1light[2].i = 0.0  light[2].a =  0  light[2].b = 0  *1light[3].i = 0.0  light[3].a =  0  light[3].b = 0  *missilePower = 10  *missileAcceleration = 0.5  *
startY = 9 *// grenades  *grenDelay = 250  *numGren = 6  *// missiles  *missDelay = 250  *numMiss = 4  *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
     }� "�g��   ����   1��� #   � �              8 � � 
          
�7�`      "�g��     (�7wa = 0  � � � � 
          
-�� "����   (!-object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (�-end � � � � � �  
     }� "�h��   � � 
          
�Z�� "�h��   (�Zwa = 0  � � ��ffff  
     }� 9 "�B   � �              8 � � 
          
$��      "����   ($object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (~$end � � � � � �  
     }� "�k��   � � 
          
�f�� "�k��   (�fwa = 0  � �   ����  
     }� 9 "�B   � �              8 � � 
          
'��      "����   ($'object Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (�'end � � � � � �  
     }� "�A�|   � � 
          
��� #   (��floorTemplate = bspFloorFrame *baseHeight = 0 *wallHeight = 0 *wallYon = 40 � � ����33  
     }� 9 "�L   � �              8 � � � �      "��   ����   a   1 y � #   � �    ��        h � � � 
                 
  �  �      "��      	 (  �adjust SkyColor end � � � � � �  
     }� "��   DD     i Z � #   � �    DDDDDD    h Z � � � 
                 
  � *	      "��   *adjust GroundColor end � � � � � �  
     }� "����     ff   a{B�k, < #   � �       h, < � � 
                 
�L��      "���   ,  	Helvetica   (�Lobject Incarnator   *y = startY end � � � � � �  
     }� "����     ff   aF.o � < #   � �       h � < � � 
                 
?%�      #   (?object Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   a7j`� < < #   � �       h < < � � 
                 
,gM�      #   (4gobject Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   a(KQt � < #   � �       h � < � � 
                 
.BO�      #   (6Bobject Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   ar-� x < #   � �       h x < � � 
                 
m%�      #   (mobject Incarnator *  y = startY *end  � � � � � �  
     }� "����     ff   asg��   < #   � �       h   < � � 
                 
xk��      "����   (�kobject Incarnator *  y = startY *end  � � � � � �  
     }� "���d   #   � �       ` ��:C � Z � � 
          
�gk      "���d     (#�object Sound  *  y = 50 *  sound = snArcticWind *  isAmbient = true *  loopCount = -1 *  volume = 80  *end  � � � �  
     }� " L    � �   @   ����     #_��             #   � �   @   ����     �{V             #   � �   @   ����      �i�i             #   � �   @   ����    P���             #   � �   @   ����    P����             #   � �   @   ����    P ��� � �             "����   a6Vb} < < #   � �       h < < � � 
          
QG��      "����     ����   (YGobject Teleporter *  group = @center y = 2*startY *  activeRange = 200  *  deadRange =  55  *  destGroup = @recenter  *  shape = 0  *end  *object Teleporter  *  y = startY * group = @recenter *  shape = bspGroundStar  *  speed = 0  *end  *object Hologram end  � � � � � �         
     }� "	��	   ��     a3�~� � Z #   � �              h � Z � � 
          
k� yf      "	��      
 (u�object Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )?blue )Close  (E�   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )?blue )Close  (��   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )?blue )Close  (<�end *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )?Blue (��   n = kTeamScore1  *
   out = @ ).blue )Wins (��end *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )Wins (��   showEveryone = true  *   text = "  ).Blue ) team wins!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true *'   text = "Yellow team has reached the   *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *+   text = "Yellow team has only 10 seconds   *to go!"  *end  � � � � � �  
     }� "	��   ��     a<��� � Z #   � �              h � Z � � 
          
u��s      "	��   (�object Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ).blue )At40 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (��	   n = 50 *
   out = @ ).blue )At50 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (�	   n = 60 *
   out = @ ).blue )At60 (9�   restart = false  *end  *object Counter *   in = @add1  )?Blue (m�	   n = 70 *
   out = @ ).blue )At70 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (��	   n = 80 *
   out = @ ).blue )At80 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (	�	   n = 90 *
   out = @ ).blue )At90 (#�   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At10 (q�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 10 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  ).Blue ) team has 20 seconds!" ( �end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  ).Blue ) team has 30 seconds!" (h�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At40 (��   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 40 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At50 (�   showEveryone = true  *   text = "  ).Blue ) team has reached the halfway  (+�mark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At60 (y�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 60 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At70 (��   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 70 seconds!" (�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At80 (I�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 80 seconds!" (p�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At90 (��   showEveryone = true  *   text = "  ).Blue ) team has only 10 seconds to   (��go!"  *end  � � � � � �  
     }� "	��   ��     a5 �� � � Z #   � �              h � Z � � 
          
f ��a      "	��   (p �object Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *$   text = "Red team has reached the  *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
     }� "	���   ��     a-gx� � Z #   � �              h � Z � � 
          
bp      "	���   (lpobject Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
     }� "	���   ��     a&Rq� � Z #   � �              h � Z � � 
          
\]�l      "	���   (f]object Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
     }� "��b   ��ffff    A�B #   � �              H � � 
         ��ffff 1D��~         
D��~      #       (N�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
     }� "���   ����33 A��� #   � �              H � � 
         ����33 1Dq�         
Dq�      #   (Nqobject WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
     }� "���     ���� A�e
� #   � �              H � � 
           ���� 1D.��         
D.��      #   (N.object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
     }� "���[   � � "����   ��     aT-� , #   � �    ����      h , � � 
                 
�wq      "���    	 (�(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (l�end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (��end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (�end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (t�end � � � � � �  
     }� "�$��   ��     a���+J, #   � �    ����      hJ, � � 
                 
�.      "�.��   (�.(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (.end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (f.end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (�.end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (.end � � � � � �  
     }� "���   ��     a���� Z, #   � �    ����      h Z, � � 
                 
� /      "�O��   (� (unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay ($ end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (� end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (� end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (, end � � � � � �  
     }� "�Y�[   ��     a��� �, #   � �    ����      h �, � � 
                 
��A�      "�a�[   (��(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (6�end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (��end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (��end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (>�end � � � � � �  
     }� "�Q��   ��     a���+, #   � �    ����33    h, � � 
                 
�59!      "�Y��   (�5(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (.5end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (�5end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (�5end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (65end � � � � � �  
     }� "���+   ��     alT�� �, #   � �    ����33    h �, � � 
                 
1��w      #   (9�(unique mShow1 mTaken1 mSwitch mShow2 end  *'object And in = @start out = mShow2 end  *object Timer +$in = mTaken1  *out = mShow1 *wait = grenDelay (��end *!object Goody +$shape = bspMissile  *y = 2  *missiles = numMiss *start = mShow1 *out = mSwitch  (��end *!object Timer +$in = mSwitch  *out = mShow2 *wait = missDelay (6�end *object Goody +$shape = bspGrenade  *y = 2  *grenades = numGren *start = mShow2 *out = mTaken1  (��end � � � � � � � �  
     }� "���h   ����33 aU��+T < #   � �              hT < � � 
          
]���      "���h       (i�object Walker +$lives = kDefaultLives *team = 2 *
y = startY *isTarget = false (��end � � � � � �  
     }� "����   ����33 aU<��J < #   � �              hJ < � � 
          
]X�       "����   (iXobject Walker +$lives = kDefaultLives *team = 2 *
y = startY *isTarget = false (�Xend � � � � � �  
     }� "���>   ����33 aU��@ < #   � �              h@ < � � 
          
]��p      "���H   (i�object Walker +$lives = kDefaultLives *team = 2 *
y = startY *isTarget = false (��end � � � � � �  
     }� "�>��   ����33 a�,�p U < #   � �              h U < � � 
          
�d      "�?��   (�dobject Walker +$lives = kDefaultLives *team = 6 *
y = startY *isTarget = false (dend � � � � � �  
     }� "�,�   ����33 a�u�� i < #   � �              h i < � � 
          
��U      "�,�-   (��object Walker +$lives = kDefaultLives *team = 6 *
y = startY *isTarget = false (�end � � � � � �  
     }� "���   ����33 a�Z  i < #   � �              h i < � � 
          
<��      "����   (Hobject Walker +$lives = kDefaultLives *team = 6 *
y = startY *isTarget = false (�end � � � � � �  
     }� "�8�K   ����33 a�c�� � < #   � �              h � < � � 
          
��,s      "�L�K   (��object Walker +$lives = kDefaultLives *team = 3 *
y = startY *isTarget = false ((�end � � � � � �  
     }� "����   ����33 a�L� � < #   � �              h � < � � 
          
A��      "����   +N%object Walker +$lives = kDefaultLives *team = 3 *
y = startY *isTarget = false (�end � � � � � �  
     }� "�%�   ����33 a��] � < #   � �              h � < � � 
          
��)      "�7�   +h&object Walker +$lives = kDefaultLives *team = 3 *
y = startY *isTarget = false (�end � � � � �  TT����pg � ������  ��  g  p         
����pg       
 "�� @   � �    � � 
         ���  
 \ 4 "���   , 	 Geneva     
 .      nene +f5designer ="Cowboy <jmcald@destiny.esd105.wednet.edu>" *�information ="Scripting by Hybrid. Minimum requirements for a game - renders very fast. Teams must be red, blue or yellow.  Up to 3 people on one team. First team to reach 100 seconds in the center square wins."  *missilePower = 1.8 *missileAcceleration = 0.3  *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
    ne   "���e   � � 
          
E;Ud "����     (Q;wa = 0  � �   ����  
    ne 1T�� "Q   � �              8 � � 
          
r���      "���|   (~�object Field  +$
deltaY = 0  *	mask = T6  *stop = @sixteen  *start = @fifteen *enter = @team6 (��end � � � � � �  
    ne "���e   ����   9 #   � �              8 � � 
          
A'QP      "����   (M'wa = 0  � � � � 
          
f�� "����   (robject Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team2 (�end � � � � � �  
    ne "���e   � � 
          
B�R "���p   (N�wa = 0  � � ��ffff  
    ne 9 "N   � �              8 � � 
          
u���      "���y   +	3object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (��end � � � � � �  
    ne "����   � � #   ����33 a�"I � < #   � �              h � < � � 
          
mg      "����   (#mobject Walker +$lives = kDefaultLives *team = 3 *y = 30 (cmend � � � � � �  
    ne "���Z   ����33 aNr�� � < #   � �              h � < � � 
          
���      "���Z   +m(object Walker +$lives = kDefaultLives *team = 3 *y = 30 (��end � � � � � �  
    ne "�^��   ����33 a��� � < #   � �              h � < � � 
          
�C@�      "�p��   +i1object Walker +$lives = kDefaultLives *team = 3 *y = 30 (<Cend � � � � � � � �  
    ne "���   � � "����   ����33 a�N � i < #   � �              h i < � � 
          
�R.      "���   (�object Walker +$lives = kDefaultLives *team = 6 *y = 30 (N�end � � � � � �  
    ne "�Y�   ����33 a�t�� i < #   � �              h i < � � 
          
��)T      "�Y�,   (��object Walker +$lives = kDefaultLives *team = 6 *y = 30 (%�end � � � � � �  
    ne "����   ����33 aM��! i < #   � �              h i < � � 
          
k��      "����   (wobject Walker +$lives = kDefaultLives *team = 6 *y = 30 (�end � � � � � � � �  
    ne "���%   � � "���E   ����33 a0�uJ < #   � �              hJ < � � 
          
8��p      "���H   (D�object Walker +$lives = kDefaultLives *team = 2 *y = 30 (��end � � � � � �  
    ne "����   ����33 a0u{J < #   � �              hJ < � � 
          
88��      "����   (D8object Walker +$lives = kDefaultLives *team = 2 *y = 30 (�8end � � � � � �  
    ne "���%   ����33 a0�u�J < #   � �              hJ < � � 
          
8��P      "���(   (D�object Walker +$lives = kDefaultLives *team = 2 *y = 30 (��end � � � � � � � �  
    ne "���e   � � 
          
�R� "���s     ff�� (�floorTemplate = bspFloorFrame *wa = 0 *wallHeight = 0 *wallYon = 60 � �         
    ne 1T�� "N5   � �       8 � � � �      "���w   ��ff   af�� � Z #   � �              h � Z � � 
          
|�!�      "���{       �� ,  	Helvetica    	 +6object Timer  +$in = @switchammo1 *out = @makeammo2 *	wait = 75  (��end *!object Goody +$shape = bspMissile  *y = 2  *	speed = 2  *missiles = 8 *start = @makeammo2 *out = @switchagain1  (�end � � � � � �         
    ne "�.��   ����33 a�4�[ < < #   � �       h < < � � 
                 
�%W�      "�=��     ���� (�%object Teleporter *  group = @center  *  y =18  *  activeRange = 230  *  deadRange = 75 *  destGroup = @recenter  *  shape = 0  *end  *object Teleporter  *  y = 9  * group = @recenter *  shape = 0  *  speed = 0  *end  � � � � � �         
    ne "��   ����   a   1 { � #   � �      ����    h � � � 
                 
  �  �      "��   , 	 Monaco    (  �adjust SkyColor end � � � � � �  
    ne "��   ������ i Z � #   � �    ������    	         h Z � � � 
                 
  � *      "��   *adjust GroundColor end � � � � � �  
    ne 	�������� "����   #   � �       `�aH� � Z � � 
          
(Lu�      "����   (1Lobject Sound  *  y = 50 *  sound = snArcticWind *  isAmbient = true *  loopCount = -1 *  volume = 80  *end  � � � � � �  
    ne "���   ��     a%lp� � Z #   � �              h � Z � � 
          
]vkJ      "���      
 (gvobject Timer  *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @yellowClose *   out[1] = @  )?blue )Close  (7v   out[2] = @redClose *end  *object Timer *   in = @team2 *   wait = 2  *   out[0] = @  )?blue )Close  (�v   out[1] = @redClose *end  *object Timer *   in = @team6 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @redClose  *end  *object Timer *   in = @team3 *   wait = 2  *   out[0] = @yellowClose *   out[1] = @  )?blue )Close  (.vend *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Counter *   in = @add1  )?Blue (�v   n = kTeamScore1  *
   out = @ ).blue )Wins (�vend *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )Wins (�v   showEveryone = true  *   text = "  ).Blue ) team wins!" (�vend *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true *'   text = "Yellow team has reached the   *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *+   text = "Yellow team has only 10 seconds   *to go!"  *end  � � � � � �  
    ne "���   ��     a.qy� � Z #   � �              h � Z � � 
          
g��W      "��   (q�object Counter  *   in = @add1Blue  *	   n = 10  *   out = @blueAt10 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 20  *   out = @blueAt20 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 30  *   out = @blueAt30 *   restart = false *end  *object Counter *   in = @add1Blue  *	   n = 40  *
   out = @ ).blue )At40 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (��	   n = 50 *
   out = @ ).blue )At50 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (�	   n = 60 *
   out = @ ).blue )At60 (+�   restart = false  *end  *object Counter *   in = @add1  )?Blue (_�	   n = 70 *
   out = @ ).blue )At70 (y�   restart = false  *end  *object Counter *   in = @add1  )?Blue (��	   n = 80 *
   out = @ ).blue )At80 (��   restart = false  *end  *object Counter *   in = @add1  )?Blue (��	   n = 90 *
   out = @ ).blue )At90 (�   restart = false  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At10 (c�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 10 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt20  *   showEveryone = true *   mask = T6 *   text = "  ).Blue ) team has 20 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @blueAt30  *   showEveryone = true *   mask = T6 *   text = "  ).Blue ) team has 30 seconds!" (Z�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At40 (��   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 40 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At50 (�   showEveryone = true  *   text = "  ).Blue ) team has reached the halfway  (�mark!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At60 (k�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 60 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At70 (��   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 70 seconds!" (��end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At80 (;�   showEveryone = true  *   mask = T6 *   text = "  ).Blue ) team has 80 seconds!" (b�end *object Text  *   sound = kSnText *   volume = kTextVolume  *	   in = @  ))blue )At90 (��   showEveryone = true  *   text = "  ).Blue ) team has only 10 seconds to   (��go!"  *end  � � � � � �  
    ne "���   ��     a' pr � � Z #   � �              h � Z � � 
          
X z�E      "���   (b zobject Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *$   text = "Red team has reached the  *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
    ne "����     ff   aE7n`J < #   � �       hJ < � � 
                 
OJp�      "����      	 (WJobject Incarnator   *y = 9  *end  � � � � � �  
    ne "���   ��     aKj� � Z #   � �              h � Z � � 
          
TTq�      "���      
 (^Tobject Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
    ne "���   ��     a6c� � Z #   � �              h � Z � � 
          
NA�P      "���   (XAobject Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true *0   text = "HECTORS must be red, yellow or blue." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  � � � � � �  
    ne "��E   ��ffff    A���% #   � �              H � � 
         ��ffff 15��a         
5��a      #       (?�object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    ne "�,�F     ff   a���� � < #   � �       h � < � � 
                 
���      #    	 (��object Incarnator *  y = 9  *end  � � � � � �  
    ne "���   ����33 A���� #   � �              H � � 
         ����33 15T��         
5T��      #     (?Tobject WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    ne "���     ���� A�H�q #   � �              H � � 
           ���� 15��         
5��      #   (?object WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team6 *   didOpen = @add1Blue *   close = @blueClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    ne "����     ff   a7-` � < #   � �       h � < � � 
                 
)/J{      "����    	 (1/object Incarnator   *y = 9  *end  � � � � � �  
    ne "��     ff   a���� < < #   � �       h < < � � 
                 
����      #   (��object Incarnator *  y = 9  *end  � � � � � �  
    ne "�z�	     ff   a��!� < #   � �       h < � � 
                 
�#�      "���   (
�object Incarnator   *y = 9  *end  � � � � � �  
    ne "���v     ff   aU�~ i < #   � �       h i < � � 
                 
v��>      "���v   (~�object Incarnator *  y = 9  *end  � � � � � �  
    ne "�v�x   ��ff   a�� � Z #   � �              h � Z � � 
          
����      "�y�|       �� (�+object And in = @start out = @makeammo3 end *object Timer +$in = @switchagain2  *out = @makeammo3 *	wait = 75  (Y�end *object Goody +$shape = bspMissile  *y = 2  *	speed = 2  *missiles = 8 *start = @makeammo3 *out = @switchammo2 (��end � � � � � �         
    ne "�|�b     ff   a��#  < #   � �       h  < � � 
                 
�)*      "���b   (�object Incarnator *  y = 9  *end  � � � � � �  
    ne "�v��   ��ff   a�t� � Z #   � �              h � Z � � 
          
�y�Q      "�y��       �� (yobject Timer  +$in = @switchammo2 *out = @makeammo4 *	wait = 75  (-yend *!object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = @makeammo4 *out = @switchagain2  (�yend � � � � � �         
    ne "����   ��ff   afs�� � Z #   � �              h � Z � � 
          
|xBP      "����       �� (�x+object And in = @start out = @makeammo1 end *object Timer +$in = @switchagain1  *out = @makeammo1 *	wait = 75  (�xend *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = @makeammo1 *out = @switchammo1 (?xend � � � � � �         
    ne "����     ff   aV�� � < #   � �       h � < � � 
                 
U{v�      #   (]{object Incarnator *  y = 9  *end  � � � � �  VV����� � ������  ��    �         
�����       
 "�� @   � �    � �  
    �   "����   � � 
          
E6U_ "����   , 	 Geneva      .      �� (Q6wa = 0  � � ����33  
    �    ASK�� "Qu   � �              H � � 
          
W��@      "���7   (c�object Field  +$
deltaY = 0  *	mask = T2  *stop = @ten  *start = @eleven  *enter = @team2 (��end � � � � � �  
    � "�	��   � � 
          
�6�_ "�	��   +�wa = 0  � � ����33  
    � A�K�� "�u   � �              H � � 
          
��@      "��7   (��object Field  +$
deltaY = 0  *	mask = T2  *stop = @ten  *start = @eleven  *enter = @team2 (�end � � � � � �  
    � "���.   � � 
          
F�V� "���   (R�wa = 0  � � ��      
    � AT�� "R�   � �              H � � 
          
`��      "����   (lobject Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (�end � � � � � �  
    � "��.   � � 
          
���� "�_�   (��wa = 0  � � ��      
    � A��� "��   � �              H � � 
          
a:��      "����   (m:object Field  +$
deltaY = 0  *	mask = T3  *stop = @sixteen  *start = @fifteen *enter = @team3 (�:end � � � � � � 
          
 5  �; "��    
 ( ? 5designer ="Cowboy <jmcald@destiny.esd105.wednet.edu>" *�information ="Scripting by Hybrid. Idea by The Head. Special thanks to Taz, for his input and infinite patience :-) Two teams of red and yellow. Up to three on a team. A team must be *alone* on  ( L�*their own* )=4 side to earn time. First team to 100 seconds wins." ( Y missilePower = 2  *missileAcceleration = 0.3  *kTeamScore1 = 100  *kInfinity = 10000  *kTeleportTheRestDelay = 20 *snYeeha = 0  *kADelay = 2  *kDefaultLives = 300  *kSnText = snTextBlip *kTextVolume = 0.3  � � � �  
    � "���   ��     aY>�� � Z #   � �              h � Z � � 
          
�H�      "
��   (�Hobject Counter  *   in = @add1Red *	   n = 10  *   out = @redAt10  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 20  *   out = @redAt20  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 30  *   out = @redAt30  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 40  *   out = @redAt40  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 50  *   out = @redAt50  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 60  *   out = @redAt60  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 70  *   out = @redAt70  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 80  *   out = @redAt80  *   restart = false *end  *object Counter *   in = @add1Red *	   n = 90  *   out = @redAt90  *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt10 *   showEveryone = true *   mask = T3 *$   text = "Red team has 10 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt20 *   showEveryone = true *   mask = T3 *$   text = "Red team has 20 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt30 *   showEveryone = true *   mask = T3 *$   text = "Red team has 30 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt40 *   showEveryone = true *   mask = T3 *$   text = "Red team has 40 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt50 *   showEveryone = true *$   text = "Red team has reached the  *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt60 *   showEveryone = true *   mask = T3 *$   text = "Red team has 60 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt70 *   showEveryone = true *   mask = T3 *$   text = "Red team has 70 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt80 *   showEveryone = true *   mask = T3 *$   text = "Red team has 80 seconds!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redAt90 *   showEveryone = true *+   text = "Red team has only 10 seconds to   *go!" *end  � � � � � �  
    � "��K   ��     aW�� � Z #   � �              h � Z � � 
          
���e      "�S   (��object Teleporter *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *   start = @timesUp  *
   win = 0 *   activeRange = kInfinity *	   y = 20  *   out = @gameEnds *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Teleporter  *   sound = 0 *   shape = 0 *   yon = 0.01  *   status = false  *	   y = 20  *   activeRange = kInfinity *   start = @teleportTheRest  *
   win = 0 *end  *object Timer *   in[0] = @yellowWins *   in[1] = @blueWins *   in[2] = @redWins  *   in[3] = @pinkWIns *   in[4] = @purpleWins *   in[5] = @cyanWins *   wait = 1  *   out = @teleportTheRest  *end  � � � � � �  
    � " �Z   ��     aZ��$ � Z #   � �              h � Z � � 
          
����      "�d   (��object Base *   in = @badHectors  *   out = @teleportTheRest  *end  *object Text  *   in = @badHectors  *   showEveryone = true **   text = "HECTORS must be red or yellow." *   sound = 0 *end  *object Goody *   out = @badHectors *   shape = bspW3x3 *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 4  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 5  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 1  *   defaultLives = 0  *end  *object Walker  *   team = 6  *   defaultLives = 0  *end  *object Walker  *   team = 6  *   defaultLives = 0  *end  *object Walker  *   team = 6  *   defaultLives = 0  *end  � � � � � �  
    � "	���   ��ffff A�� #   � �              H � � 
         ��ffff 1[y         
[y      #   ,  	Helvetica     (eyobject WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team3 *   didOpen = @add1Red  *   close = @redClose *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    � "	���   ����33 AE"n #   � �              H � � 
         ����33 1[�         
[�      #   (eobject WallDoor *   deltaY = 3  *   openSpeed = 20  *   closeSpeed = 100  *   open = @team2 *   didOpen = @add1Yellow *   close = @yellowClose  *   openSound = 0 *   stopSound = 0 *   closeSound = 0  *   hitSound = 0  *end  � � � � � �  
    � "�a   ��     a`��+ � Z #   � �              h � Z � � 
          
��_�      "�j      
 (��// red timer  *object Timer *   in[0] = @start  *   in[1] = @fifteen  *   wait = 15 *   out[0] = @fifteen *end  *object Timer *   in = @fifteen *   wait = 1  *   out = @sixteen  *end  *object Timer *   in = @fifteen *   wait = 10 *   out[0] = @redClose  *end  *object Timer *   in = @team4 *   wait = 2  *   out[0] = @redClose  *end  *object Counter *   in = @add1Red *   n = kTeamScore1 *   out = @redWins  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @redWins *   showEveryone = true *   text = "Red team wins!" *end  � � � � � �  
    � "�\   ����   a^��& � Z #   � �              h � Z � � 
          
����      "�e   (��// yellow timer *object Timer *   in[0] = @start  *   in[1] = @ten  *   wait = 15 *   out[0] = @ten *end  *object Timer *   in = @ten *   wait = 1  *   out = @eleven *end  *object Timer *   in = @ten *   wait = 10 *   out[0] = @yellowClose *end  *object Timer *   in = @team5 *   wait = 2  *   out[0] = @yellowClose *end  *object Counter *   in = @add1Yellow  *   n = kTeamScore1 *   out = @yellowWins *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowWins  *   showEveryone = true *   text = "Yellow team wins!"  *end  *object Counter *   in = @add1Yellow  *	   n = 10  *   out = @yellowAt10 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 20  *   out = @yellowAt20 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 30  *   out = @yellowAt30 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 40  *   out = @yellowAt40 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 50  *   out = @yellowAt50 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 60  *   out = @yellowAt60 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 70  *   out = @yellowAt70 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 80  *   out = @yellowAt80 *   restart = false *end  *object Counter *   in = @add1Yellow  *	   n = 90  *   out = @yellowAt90 *   restart = false *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt10  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 10 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt20  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 20 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt30  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 30 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt40  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 40 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt50  *   showEveryone = true *'   text = "Yellow team has reached the   *halfway mark!" *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt60  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 60 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt70  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 70 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt80  *   showEveryone = true *   mask = T2 *'   text = "Yellow team has 80 seconds!"  *end  *object Text  *   sound = kSnText *   volume = kTextVolume  *   in = @yellowAt90  *   showEveryone = true *+   text = "Yellow team has only 10 seconds   *to go!"  *end  � � � � � � 
          
 & X "���     (" wheight = 15  � � � �  
    � "��   ����   a   1 { � #   � �      ����    h � � � 
                 
  �  �      "��   , 	 Monaco     	 (  �adjust SkyColor end � � � �   ����  
    � i Z � " 
 s   � �    ������    	         h Z � � � 
                 
  � *      "��   *adjust GroundColor end � � � �  
    � 	�������� "���   #   � �       ` ��9� � Z � � 
          
�f
      "���   ("�object Sound  *  y = 50 *  sound = snArcticWind *  isAmbient = true *  loopCount = -1 *  volume = 80  *end  � � � � � � 
          
 �5� "�{��       (5wallHeight = wheight  � � ������  
    � 1�U� "  �   � �              8 � � 
          
%�5d      "���[   (1�wallHeight = wheight  � � ������  
    � 1��%� "  �   � �              8 � � 
          
�D��      "�A��   (�DwallHeight = wheight  � � ������  
    � 1C%� "  �   � �              8 � � 
          
 ��=      "�{�4   (�wallHeight = wheight  � � ������  
    � 1��D "  �   � �              8 � � 
          
� ��      "��   (� �wallHeight = wheight  � � ������  
    � 1 �% "  �   � �              8 � �      "���   � � 
          
���
 "�N�   (��wa = 0  *wallHeight = 0 � � ��      
    � 1T� "��   � �              8 � � � �      "����   � � 
          
�6�� "�N��   (�6wa = 0  *wallHeight = 0 � � ��      
    � 1T��� "�6   � �              8 � � � �      "���"   � � 
          
���� "�N�9   (��wa = 0  *wallHeight = 0 � � ����33  
    � 1T��� "q�   � �              8 � � � � 
          
���=      "�J�4   (��wallHeight = wheight  � � ������  
    � 1��uD "  �   � �              8 � � 
          
���=      "�+�4   *�wallHeight = wheight � � ������  
    � 1��MD "  �   � �              8 � � 
          
�-��      "���   (�-wallHeight = wheight  � � ����33  
    � 1���� "  �   � �              8 � � 
          
���       "��=    	 (��object Field  *deltaY = .1  *deltaX = -.4 *visible = true *end  � � � � 
          
~-�� "����     (�-wallHeight = wheight  � � ����33  
    � 1t��� "  �   � �              8 � � 
          
��       "���=    	 (��object Field  *deltaY = .1  *deltaX = -.4 *visible = true *end  � � � � 
          
���= "��4     (��wallHeight = wheight  � � ������  
    � 1��%D "  �   � �              8 � � 
          
V-f�      "����   (b-wallHeight = wheight  � � ����33  
    � 1L��� "  �   � �              8 � � 
          
W��       "���=    	 (a�object Field  *deltaY = .1  *deltaX = -.4 *visible = true *end  � � � � 
          
 �� "�{�     (wallHeight = wheight  � � ������  
    � 1�� "  �   � �              8 � � 
          
���      "�J�   *�wallHeight = wheight � � ������  
    � 1�u� "  �   � �              8 � � 
          
���      "�+�   *�wallHeight = wheight � � ������  
    � 1�M� "  �   � �              8 � � 
          
� ��V      "��M   (� �wallHeight = wheight  � � ��      
    � 1�S�� "  �   � �              8 � � 
          
�]��      "���    	 (�]object Field  *deltaY = .1  *deltaX = .4  *visible = true *end  � � � � 
          
~ ��V "���M     (� �wallHeight = wheight  � � ��      
    � 1tS�� "  �   � �              8 � � 
          
]��      "����    	 (�]object Field  *deltaY = .1  *deltaX = .4  *visible = true *end  � � � � 
          
��� "��     (�wallHeight = wheight  � � ������  
    � 1�%� "  �   � �              8 � � 
          
V �fV      "���M   (b �wallHeight = wheight  � � ��      
    � 1LS�� "  �   � �              8 � � 
          
W]��      "����    	 (a]object Field  *deltaY = .1  *deltaX = .4  *visible = true *end  � � � �  
    � "����   � � 
          
�Q�D "�N��     (�Qwa = 0  *wallHeight = 0 � � ����33  
    � 1T9�L "q�   � �              8 � � � �      "���   ����33 a��: < < #   � �       h < < � � 
                 
�8�      "���     ����    	 (�object Teleporter *  group = @center  *  y =-8  *  activeRange = 150  *  deadRange =  80  *  destGroup = @recenter  *  shape = 0  *end  *object Teleporter  *  y = 9  * group = @recenter *  shape = 0  *  speed = 0  *end  � � � � � �         
    � "�]��   ��ff   a��: � Z #   � �              h � Z � � 
          
���      "�`��       �� (�object Timer  +$in = @switchammo1 *out = @makeammo2 *	wait = 25  (end *!object Goody +$shape = bspMissile  *y = 2  *	speed = 2  *missiles = 8 *start = @makeammo2 *out = @switchagain1  (�end � � � � � �         
    � "����   � � 
          
6V	 #       (Bwa = 0  *wallHeight = 0 � �  
    � 1T�: "Rq   � �    ������    	         8 � � � �             "����   ��ff   	�������� aDi: � Z #   � �              h � Z � � 
          
Z��      "����   wwwwww    	 (bobject Timer  +$in = @switchammo2 *out = @makeammo4 *	wait = 25  (�end *!object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = @makeammo4 *out = @switchagain2  (�end � � � � � �         
    � "�3�   ��     a��2 � < #   � �              h � < � � 
          
� �?C      #     (� �object Walker *   lives = kDefaultLives *   team = 3  *	   y = 10  *end  *object Incarnator  +$	mask = T3 *y = 10 (; �end � � � � � �  
    � "����     ��   a_�B < < #   � �              h < < � � 
          
t6 �      "����   (~6object Walker +$lives = kDefaultLives *team = 2 *y = 10 (�6end *object Incarnator  +$	mask = T2 *y = 10 (�6end � � � � � �  
    � "�Y��   ��ff   a��: � Z #   � �              h � Z � � 
          
���      "�\��       ��  	 (�+object And in = @start out = @makeammo3 end *object Timer +$in = @switchagain2  *out = @makeammo3 *	wait = 25  (<end *object Goody +$shape = bspMissile  *y = 2  *	speed = 2  *missiles = 8 *start = @makeammo3 *out = @switchammo2 (�end � � � � � �         
    � "����   ��ff   aHm: � Z #   � �              h � Z � � 
          
^$�      "����       �� (|+object And in = @start out = @makeammo1 end *object Timer +$in = @switchagain1  *out = @makeammo1 *	wait = 25  (�end *object Goody +$shape = bspGrenade  *y = 2  *	speed = 2  *grenades = 12  *start = @makeammo1 *out = @switchammo1 (!end � � � � � �         
    � "�7��     ��   a��B < < #   � �              h < < � � 
          
�6N�      "�B��     (�6object Walker +$lives = kDefaultLives *team = 2 *y = 10 (6end *object Incarnator  +$	mask = T2 *y = 10 (J6end � � � � � �  
    � "�
�   ��     a��3 � < #   � �              h � < � � 
          
� �D      #   (� �object Walker *   lives = kDefaultLives *   team = 3  *	   y = 10  *end  *object Incarnator  +$	mask = T3 *y = 10 ( �end � � � � � �  
    � "���     ��   a��B < < #   � �              h < < � � 
          
�6&�      "���   (�6object Walker +$lives = kDefaultLives *team = 2 *y = 10 (�6end *object Incarnator  +$	mask = T2 *y = 10 ("6end � � � � � �  
    � "���   ��     a]�2 � < #   � �              h � < � � 
          
d ��C      #   (n �object Walker *   lives = kDefaultLives *   team = 3  *	   y = 10  *end  *object Incarnator  +$	mask = T3 *y = 10 (� �end � � � � � �  
    � "����   � � 
          
F6V_ "����     (R6wa = 0  � � ����33  
    � ATK�� "Ru   � �              H � � 
          
X��@      "���7   (d�object Field  +$
deltaY = 0  *	mask = T3  *stop = @ten  *start = @eleven  *enter = @team5 (��end � � � � � �  
    � "�
��   � � 
          
�6�_ "�
��   +�wa = 0  � � ����33  
    � A�K�� "�u   � �              H � � 
          
��@      "��7   (��object Field  +$
deltaY = 0  *	mask = T3  *stop = @ten  *start = @eleven  *enter = @team5 (�end � � � � � �  
    � "���.   � � 
          
F�V� "���   (R�wa = 0  � � ��      
    � AT�� "R�   � �              H � � 
          
`��      "����   (lobject Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team4 (�end � � � � � �  
    � "��.   � � 
          
���� "�_�   (��wa = 0  � � ��      
    � A��� "��   � �              H � � 
          
a:��      "����   (m:object Field  +$
deltaY = 0  *	mask = T2  *stop = @sixteen  *start = @fifteen *enter = @team4 (�:end � � � � �    �M �M  ��;�DN    � TMPL  "PICT 	 :LEDI   �HULL   � �      �9< � L �`�9 �  ��:� � �s�:� j  fj�9��   �h�9� � " p�9�� Q aH�: � v ,5�:� 5 �%�:� B K5�8@� \   ��9� ��� �O�:  � � �;�8�LEDIbaghdad2.pictshowdown2.pictgeezer koth splashnothing.pict	line.pictHULL
rodeo.pictkingring.pictmirage.pictburnedagain.pict	sand.pict
light.hull