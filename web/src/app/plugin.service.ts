import { Injectable } from '@angular/core';
import { Plugin } from './shared/plugin';
import { Observable, of, forkJoin } from 'rxjs';
import { map, switchMap } from "rxjs/operators";
import { HttpClient } from '@angular/common/http';
import { ValueTransformer } from '@angular/compiler/src/util';


export interface CommandListResponse {
  plugin: Plugin;
  commands: string[];
}

export interface PluginListResponse {
  master: boolean;
  plugins: Plugin[];
}

@Injectable({
  providedIn: 'root'
})
export class PluginService {

  constructor(
    private http: HttpClient) { }

  commands: Observable<string[]>;
  plugins: Observable<Plugin[]>;
  monitors: Observable<Plugin[]>;

  COMMAND_LIST_URI: string = '/command-list';
  PLUGIN_LIST_URI: string = '/plugin-list';

  getCommandList() : Observable<string[]> {
    return this.commands;
  }
  getPluginList() : Observable<Plugin[]> {
    return this.plugins;
  }

  getMonitorList() : Observable<Plugin[]> {
    return this.monitors;
  }

  fetchPluginList(commands: string[]) : Observable<Plugin[]> {
    var httpRequests = commands.map(cmd => this.http.get<PluginListResponse>(cmd));
    const joinedRequests = forkJoin(httpRequests);
    return joinedRequests.pipe(map((responses: PluginListResponse[]) => {
      var masterList = responses.find(function(element: PluginListResponse) { return element.master == true; });
      if(masterList === undefined) {
        return []; // no master list found.
      }
      return masterList.plugins;
    }));
  }

  refresh(hostname: string) : void {
    // fetch commands
    this.commands = this.http.get<CommandListResponse>(hostname + this.COMMAND_LIST_URI).pipe(
      map((response: CommandListResponse) => {
        return response.commands;
      })
    );

    // fetch all plugins
    this.plugins = this.commands.pipe(
      switchMap((commands: string[]) => {
        var pluginListCommands = commands.filter(cmd => cmd.includes(this.PLUGIN_LIST_URI));
        var fullUrls = pluginListCommands.map(val => hostname + val);
        return this.fetchPluginList(fullUrls);
      })
    );

    // filter monitors
    this.monitors = this.plugins.pipe(
      map((plugins: Plugin[]) => {
        return plugins.filter(plugin => plugin.name === "monitor");
      })
    );
  }
}
