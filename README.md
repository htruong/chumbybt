Huan's Public Bittorrent Sync + Webkit Chumby release
=============

You would have to do all the customizations on on-device folder. 

Edit network_config for your wifi network if you want to connect the Chumby to your wifi network. The sample config files are on `on_device/psp`. Copy `.sample` to create our own.

You will need to modify at least `network_config` and `htpasswd` for this whole thing to work.

If you need to mount a nfs share, edit `nfsmount` accordingly.

Then, use ./repack-update.sh to pack the new update pack, it will unpack the usual chumby stuff overlay our customizations on top of it.

Copy the copy-to-flash-drive folder to the ROOT of a USB flash drive.

Plug the USB flash drive to the back of the chumby and press THE SCREEN when the chumby is booting up. You'll be presented with a choice of upgrading the chumby. 

Enjoy.