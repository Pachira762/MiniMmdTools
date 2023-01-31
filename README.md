# Mini MMD Tools

**Mini MMD Tools** はUnrean EngineでMikuMikuDance関連のファイルを読み込むためのプラグインです。  
VMDファイルのモーフとカメラのインポートに対応しています。  
Blenderプラグインの **MMD Tools** や **Send to Unreal** との併用を想定しています。

## 注意

配布されたモデルやモーションの利用規約をよく読み使用してください。

## 関連ツール

[MMD Tools](https://mmd-blender.fandom.com/ja/wiki/MMD_Tools)  
[Send to Unreal](https://epicgames.github.io/BlenderTools/send2ue/)  
[Kawaii Physics](https://github.com/pafuhana1213/KawaiiPhysics)  

## インストール

プロジェクトのPluginsフォルダに入れてください。

## 使い方

### モーフインポーター

コンテンツブラウザにVMDファイルをドラッグアンドドロップします。  
インポートされたアイテムを右クリックして `スクリプト化されたアセットアクション` > `Apply to Anim Sequence` を実行してモーフを適応させます。

### カメラインポーター

コンテンツブラウザにVMDファイルをドラッグアンドドロップします。

### カメラ

`Plugins\MiniMmdTools C++コンテンツ\MiniMmdTools\Public` フォルダ内の *MmdCameraActor* をレベルに配置しレベルシーケンサに追加します。  
タイムラインの最初に *Frame* キーを *0.0* のリニア補間で打ちます。  
タイムラインの最後に *Frame* キーを 1000フレーム目なら1000.0 のようにリニア補間で打ちます。  
*CameraSequence* に読み込んだVMDファイルを選択します。

## Tips

Move Render Queue の Temporal Sample を有効にしてレンダリングする場合は *MmdCameraActor* の *Use Temporal Sampling* をオンにして *Output Frame Rate* を設定してください。 

## 連絡

[Twitter Pachira](https://twitter.com/pachira762)
