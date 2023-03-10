????  -? Code 
SourceFile ConstantValue 
Exceptions LineNumberTable LocalVariableTable CoolHeadLines  java/applet/Applet 	 Ljava/lang/Thread; m_CoolHeadLines  	   ()V stop   java/lang/Thread 
   this LCoolHeadLines; Ljava/awt/Dimension; m_dimAppletSize  	   I height   java/awt/Dimension 	   m_iFontHeight ! 	  " y iY Z m_fStoppedScrolling ' &	  ( m_iOldMessageSelected * 	  + ()Ljava/awt/Graphics; getGraphics . - java/awt/Component 0
 1 / m_iOldMessageSelectedYOffset 3 	  4 Ljava/awt/FontMetrics; m_fm 7 6	  8 ()I 
getDescent ; : java/awt/FontMetrics =
 > < Ljava/awt/Color; m_colTextColor A @	  B )(Ljava/awt/Graphics;IIILjava/awt/Color;)V 
drawString E D
  F evt Ljava/awt/Event; x Ljava/util/Vector; 
m_vectData L K	  M (I)Ljava/lang/Object; 	elementAt P O java/util/Vector R
 S Q TextJump U ()Ljava/lang/String; 	getString X W
 V Y iIndex t 
LTextJump; [Ljava/lang/String; ^ java/lang/String ` 	BackColor b String d Background Color f 	TextColor h 
Text Color j HiliteTextColor l Hilited Text Color n ScrollDelay p int r msec delay between each scroll t MessageDelay v sec delay between each message x NumItems z Number of items | 	URLPrefix ~ !Prefix for all .htm or .asp files ? info [[Ljava/lang/String; (C)I 	charWidth ? ?
 > ? (II)I min ? ? java/lang/Math ?
 ? ? Ljava/awt/Font; m_font ? ?	  ? (Ljava/awt/Font;)V setFont ? ? java/awt/Graphics ?
 ? ? (Ljava/awt/Color;)V setColor ? ?
 ? ? (Ljava/lang/String;II)V E ?
 ? ? (IIII)V fillOval ? ?
 ? ? g Ljava/awt/Graphics; str Ljava/lang/String; col 
iFontWidth 
iOvalWidth (I)Ljava/lang/String; getStringFromVector ? ?
  ? :(Ljava/awt/Graphics;Ljava/lang/String;IILjava/awt/Color;)V E ?
  ? iMsg (Ljava/lang/Runnable;)V <init> ? ?
  ? start ? 
  ? drawHeadlines ? 
  ? rName: CoolHeadLines
Author: Michael Howard (mikehow@microsoft.com)
Created with Microsoft Visual J++ Version 1.1 ? ? 
 
 ? PARAM_BackColor ? ?	  ? PARAM_TextColor ? ?	  ? PARAM_HiliteTextColor ? ?	  ? PARAM_ScrollDelay ? ?	  ? PARAM_MessageDelay ? ?	  ? PARAM_NumItems ? ?	  ? PARAM_URLPrefix ? ?	  ? m_iScrollDelay ? 	  ? m_iMessageDelay ? 	  ? m_iCurrentMessage ? 	  ? java/awt/Color ? (III)V ? ?
 ? ? m_colBackColor ? @	  ? m_colHiliteTextColor ? @	  ? size ? :
 S ? scrollOneLine ? 
  ? m_iMaxMessage ? 	  ? LDoubleBuffer; m_buff ? ?	  ? DoubleBuffer ?
 ? / ()Ljava/awt/Rectangle; getSize ? ?
 ? ? java/awt/Rectangle ?	 ?  i (I)I 
getYOffset
  suspend 
  java/lang/StringBuffer
	 ? m_strURLPrefix ?	  ,(Ljava/lang/String;)Ljava/lang/StringBuffer; append
	 / getURL W
 V toString W
	 (Ljava/lang/String;)V 
showStatus
 
 ()Ljava/applet/AppletContext; getAppletContext
 
  java/net/URL" ?
#$ (Ljava/net/URL;)V showDocument'& java/applet/AppletContext)*( java/net/MalformedURLException, iYOffset 	iWhichMsg e  Ljava/net/MalformedURLException; strSampleURL java/util/StringTokenizer3
4$ 	nextToken6 W
47 (Ljava/lang/String;)I parseInt:9 java/lang/Integer<
=; strRGB st Ljava/util/StringTokenizer; iRed iGreen iBlue (J)V sleepFE
 G java/lang/InterruptedExceptionI
    Ljava/lang/InterruptedException; &(Ljava/lang/String;)Ljava/lang/String; getParameterNM
 
O $(Ljava/lang/String;)Ljava/awt/Color; getColorFromRGBRQ
 S m_iNumItemsU 	 V ()Ljava/awt/Dimension; ?X
 1Y java/awt/Font[ 	Helvetica] ? ?
\_ '(Ljava/awt/Font;)Ljava/awt/FontMetrics; getFontMetricsba
 1c getMaxDescente :
 >f 
getLeadingh :
 >i getMaxAscentk :
 >l widthn 	 o ? ?
 ?q +(Ljava/applet/Applet;Ljava/awt/Rectangle;)V ?s
 ?t J 	 ?v $ 	 ?x	 ?o fillRect{ ?
 ?| setBackground~ ?
 1
 S ? Text? (I)Ljava/lang/StringBuffer;?
	? URL? '(Ljava/lang/String;Ljava/lang/String;)V ??
 V? (Ljava/lang/Object;)V 
addElement??
 S? Ljava/io/PrintStream; out?? java/lang/System?	?? 
? println? java/io/PrintStream?
?? param r Ljava/awt/Rectangle; strText strURL ()Ljava/awt/Image; getImage??
 ?? 3(Ljava/awt/Image;IILjava/awt/image/ImageObserver;)Z 	drawImage??
 ?? j  at ?* strStatusMsg java/lang/Runnable? CoolHeadLines.java run getAppletInfo getParameterInfo ()[[Ljava/lang/String; init 	mouseMove (Ljava/awt/Event;II)Z 	mouseExit 	mouseDown !  
 ?   ? ?      c  ? ?      i  ? ?      m  ? ?      q  ? ?      w  ? ?      {  ? ?        ? @    A @    ? @    ?     ?    U     ?              L K    ? ?    7 6    !     ? ?    ' &    ?     ?     *     3            J     *? ? *? ? *? ?           ?  ?  ?  ?                  S     *? ?  d*? #l=?       
   F G                 $     %   ??     ?     C*? )? ?*? ,? 4**? 2*? ,*? ?  *? 5*? #hd*? 9? ?d`*? C? G*? ,?       * 
  ? ? 	? ? ? ? ? 5? <? A?    *    C       C H I    C J     C $    ? ?     S     *? N? T? VM,? Z?       
   # $                 [     \ ]  ??     ?     ?? _Y? aYcSYeSYgSSY? aYiSYeSYkSSY? aYmSYeSYoSSY? aYqSYsSYuSSY? aYwSYsSYySSY? aY{SYsSY}SSY? aYSYeSY?SSL+?       & 	   P  R # S 9 T O U e V { W ? X ? [        ?     ?  ? ?   E ?     ?     I*? 9W? ?6l*? #l? ?6+*? ?? ?+? ?+,`? ?+l`dd? ??             " ( 3 H    R    I       I ? ?    I ? ?    I J     I $     I ? @   > ?    / ?    E D     p     *+*? ?? ??       
        >            ? ?     ?      J      $      ? @   ?      \     "*? ? *? Y*? ?? *? ? ?? *? ??           ?  ?  ?  ? ! ?        "     ? W     -     ??           F              ?      ?     {*? ?*c? ?*i? ?*m? ?*q? ?*w? ?*{? ?*? ?*
? ?*? ?*? ?*? ,*? 5*? ?Y ? ? ?? ?? ?*? ?Y? ?? C*? ?Y ?? ?? ??       F    :           #  )  / % 5 & : 2 ? 4 D 5 H < \ = j > z ?        {      ?      ?     `*Y? ?`? ?*Y? ?*? N? ?p? ?*? ݚ <? 
*? ??*? ????**? ?? ?*? ?*? ?? ?? ?*? 9? ?d*? C? G*? ??       6    ? 
 ?  ? ! ? & ? * ? 5 ? = ? A  B T [ _        `     #     ??         j*? )? ?*?6*? ?d6*? ??	Y?
*???*? N? T? V???:*?*?!?#Y?%?+ ? :?   c f-     * 
  ? ? 	? ? ?  ? K? Q? f? h?    R    j       j H I    j J     j $    Z.    Q/   h  01  K 2 ?  RQ     ?     0?4Y+?5M,?8?>>,?8?>6,?8?>6? ?Y? ??           ? 	 ?  ?  ? # ?    >    0       0? ?  	 '@A   B    C   # D   ?      ?     2? /*? )*? ,*? 5*? ?*? )*? ??h??H? L*?K???   ' *J     * 
   ?  ?  ?  ?  ?  ?  ? * ? + ? / ?        2     + 0L  ?     ?    ?*c?PL+? **+?T? ?*i?PL+? **+?T? C*m?PL+? **+?T? ?*q?PL+? *+?>? ?*w?PL+? *+?>? ?*{?PL+? *+?>?W*?PL+? *+?**?Z? *?\Y^?`? ?***? ??d? 9**? 9?g*? 9?j`*? 9?m`? #? ?Y*? ?p*? ?  *? #`?rM*? ?Y*,?u? ?*? ?? ?*? ?? ?*? ?? ?,?w,?y,?z,? ??}**? ???**? ?  *? #l`? ?*? SY??? N>? g*?	Y?
??????P:*?	Y?
??????P:*? N? VY???????	Y?
???????*?W????       ? '   f  g  h  k  l  m ( p / q 3 r < u C v G w O z V { Z | b  i ? m ? u ? | ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?1 ?< ?A ?[ ?u ?? ?? ?? ?    >   ?     ?? ?  ? ??? > o   [ G? ? u -? ?   ?      ?     r<? %*? 2*? ???t*??W*? ׅ?H? M?*? #???*? ?? ?L+*? ???*? #t*??W+*? ?? ?+*? ?  *? ?p*? ?  *? #`?}?     #J     6   - / 0 #1 $- /4 77 J: R; T< [= b> q?    *    r      -?   $  0L  7 ; ? ?  ??    ?     ?*? )? ?*?6*? ?d6*? ,? ?*? ,? 9**? 2*? ,*? ?  *? 5*? #hd*? 9? ?d`*? C? G? *? ,? |*? ,*? 5**? 2*? ?  *? #hd*? 9? ?d`*? ?? G?	Y?
*? ????*???*? N? T? V???:*?!?? ?       v   P Q 	T U X "Y $\ +^ 0_ 4` 5a Pb Wd \e ai fl lm ro wp yq zr ?s ?x ?v ?w ?x ?y ?z ?}    H    ?       ? H I    ? J     ? $    ?.    ?/   ? ? ?      ?