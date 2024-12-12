#!/usr/bin/env perl
use strict;
use warnings;


die 'wimfc error :unable to run this script under MS Windows!' if $^O eq 'MSWin32';
die 'wimfc error :failed to recognize active shell!' unless(exists $ENV{SHELL});
die 'wimfc error :you should be using fish shell!' unless($ENV{SHELL} =~ /fish/);


my $history_file_path = "$ENV{HOME}/.local/share/fish/fish_history";
open(my $histfile, '<', $history_file_path) or die "unable to open: '$history_file_path'! $!";


#extract all actual commands
my @commands; 
while (<$histfile>)
{
    push @commands, substr($_,6)  if($_ =~ /cmd/);
};

close($histfile);




#now count how much times each command is used
our %frequencies;

sub count_command
{
    my ($command) = @_;
    if(exists $frequencies{$command}){ $frequencies{$command} += 1; }
    else { $frequencies{$command} = 1; }
}


foreach(@commands)
{
    my $is_sudo_case = $_ =~ /sudo/; #don't count sudo, so take command after it
    my $is_pipe_case = $_ =~ /\|/;   #check each command in the pipe

    my @words = split(/\s+/, $_);
 
    count_command $words[1] unless($is_sudo_case && $is_pipe_case); #simple case
    count_command $words[2] if($is_sudo_case && !$is_pipe_case); #entry contains sudo

    if($is_pipe_case)
    {
	#get rid of every sudo string in case it is in string
	my $killed_sudo = $_ =~ /sudo/? $_ =~ s/sudo//g : $_;

	#get every command in pipeline
	#so now we have list of commands
	my @subcommands = split(/\|/,$killed_sudo); 

	#now count each command from the pipeline
	foreach my $command (@subcommands)
	{
	    my @words = split(/\s+/, $_);
	    count_command $words[1];
	}	 
    }
};

#find the most frequent
my $frequent_key = (sort { $frequencies{$b} <=> $frequencies{$a} } keys %frequencies)[0];
print "$frequent_key\n";
