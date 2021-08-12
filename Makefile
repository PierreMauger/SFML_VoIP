##
## EPITECH PROJECT, 2020
## Base
## File description:
## Main Makefile
##

#################### PATH #####################
PATH_CLIENT		=	client/

PATH_SERVER		=	server/

################# COMPILATION #################
all:
	make -C	$(PATH_CLIENT)
	make -C	$(PATH_SERVER)

re:	fclean all

clean:
	@(make clean -C $(PATH_CLIENT)) > /dev/null
	@(make clean -C $(PATH_SERVER)) > /dev/null

fclean:	clean
	@(make fclean -C $(PATH_CLIENT)) > /dev/null
	@(make fclean -C $(PATH_SERVER)) > /dev/null

debug:
	@(make debug -C $(PATH_CLIENT)) > /dev/null
	@(make debug -C $(PATH_SERVER)) > /dev/null
